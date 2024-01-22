#include "graph.h"
#include "./ui_graph.h"
#include "node.h"
#include "linkline.h"
#include "myundocommand.h"

#include<random>
#include<set>
#include<stack>

#include<QRegularExpressionValidator>
#include<QMessageBox>
#include<QGraphicsScene>
#include<QGraphicsView>
#include<QGraphicsSceneMouseEvent>
#include<QKeyEvent>
#include<QPainterPath>

Graph::Graph(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Graph)
{
    ui->setupUi(this);


    QRegularExpressionValidator *rev=new QRegularExpressionValidator(QRegularExpression("(\\d{0,4}[,])*"));
    ui->numsVecLineEdit->setValidator(rev);
    ui->numsVecLineEdit->setPlaceholderText(tr("样例：1,2,3,4,5"));

    scene=new QGraphicsScene(this);
    QGraphicsView* view=new QGraphicsView(scene,ui->frame);
    scene->setSceneRect(ui->frame->rect());
    view->setRenderHint(QPainter::Antialiasing);

    scene->installEventFilter(this);

    undoStack=new QUndoStack(this);

    connect(ui->generateNodesBtn,&QPushButton::clicked,this,&Graph::generateNodes);
    connect(ui->clearNodesBtn,&QPushButton::clicked,this,&Graph::clear);
}

Graph::~Graph()
{
    delete ui;
    while(!numsQue.empty()){
        numsQue.pop();
    }
    while(!nodeQue.empty()){
        delete nodeQue.front();
        nodeQue.pop();
    }
    for(auto& l:linkLineVec){
        delete l;
        l=nullptr;
    }

    linkLineVec.clear();
    selectedNodes.clear();
    selectedLines.clear();
    undoStack->clear();
}

void Graph::getNums()
{
    if(ui->numsVecLineEdit->text().isEmpty())   return;

    auto strList=ui->numsVecLineEdit->text().split(',',Qt::SkipEmptyParts);
    for(auto &s: strList){
        numsQue.push(s.toInt());
    }
}

void Graph::generateNodes()
{
    emit ui->clearNodesBtn->click();
    while(!nodeQue.empty()){
        delete nodeQue.front();
        nodeQue.pop();
    }
    scene->clear();
    if(ui->numsVecLineEdit->text().isEmpty()){
        QMessageBox::information(this,"提示！","请输入数据集！",QMessageBox::Retry);
        return;
    }
    if(!ui->numsVecLineEdit->isModified()){
        QMessageBox::information(this,"提示！","请修改数据集！",QMessageBox::Retry);
        return;
    }

    this->getNums();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> xPos(scene->sceneRect().width()/2,100);
    std::normal_distribution<> yPos(scene->sceneRect().height()/2,100);

    while(!numsQue.empty()){
        Node* node=new Node(numsQue.front());
        auto x=xPos(gen);
        auto y=yPos(gen);
        scene->addItem(node);
        node->setPos(x,y);
        node->setToolTip(QObject::tr("(%1,%2)").arg(node->x()).arg(node->y()));
        while(!node->collidingItems().empty()){
            x=xPos(gen);
            y=yPos(gen);
            node->setPos(x,y);
        }
        nodeQue.push(node);
        numsQue.pop();
    }
}

void Graph::clear()
{
    while(!this->nodeQue.empty()){
        nodeQue.pop();
    }
    for(auto& i: linkLineVec){
        delete i;
    }
    linkLineVec.clear();
    this->scene->clear();

}

bool Graph::eventFilter(QObject *watched, QEvent *event)
{
    if(watched==this->scene){

        static QPointF startPos(0,0);
        static QPointF endPos(0,0);
        static QGraphicsRectItem selectionArea;

        static Qt::MouseButton buttonType=Qt::NoButton;
        if(event->type()==QEvent::GraphicsSceneMousePress){
            auto e=dynamic_cast<QGraphicsSceneMouseEvent*>(event);
            if(e->button()==Qt::RightButton){

                startPos=e->scenePos();
                QGraphicsLineItem *li=new QGraphicsLineItem;
                scene->addItem(li);
                li->setZValue(1);

                buttonType=Qt::MouseButton::ExtraButton1;
            }
            if(e->button()==Qt::LeftButton){
                if(scene->itemAt(e->scenePos(),QTransform())==nullptr){
                    selectionArea.setPen(QPen(Qt::DotLine));
                    scene->addItem(&selectionArea);
                    startPos=e->scenePos();
                    selectionArea.setPos(startPos);
                }
            }
        }
        if(event->type()==QEvent::GraphicsSceneMouseMove){

            auto e=dynamic_cast<QGraphicsSceneMouseEvent*>(event);
            if(buttonType==Qt::MouseButton::ExtraButton1 && e->scenePos()!=endPos){

                endPos=e->scenePos();
                QGraphicsLineItem* lineItem=dynamic_cast<QGraphicsLineItem*>(scene->items().front());
                lineItem->setLine(QLineF(startPos,endPos));
            }
            if(e->buttons()==Qt::LeftButton){
                const Node* node=this->getNodeAtPos(e->scenePos());
                //auto node=scene->itemAt(e->scenePos(),QTransform());
                if(node!=nullptr){
                    std::vector<LinkLine*> lineList=node->getLineList();
                    for(auto& l:lineList)
                        l->updateSelf();
                }
                else{
                    endPos=e->scenePos();
                    QPointF selecBottomRight=selectionArea.mapFromScene(endPos);
                    QRectF selecRect=startPos.x()<=endPos.x() ?
                                          QRectF(QPointF(0,0),selecBottomRight) : QRectF(selecBottomRight,QPointF(0,0));
                    //QRectF selecRect(QPointF(0,0),selecBottomRight);
                    //QRectF selecRect(selectionArea.mapToScene(0,0),selecBottomRight);
                    selectionArea.setRect(selecRect);
                }
            }
        }
        if(event->type()==QEvent::GraphicsSceneMouseRelease){

            auto e=dynamic_cast<QGraphicsSceneMouseEvent*>(event);
            const double distance=(startPos-endPos).manhattanLength();
            if(e->button()==Qt::RightButton && distance>0){
                endPos=e->scenePos();
                QGraphicsLineItem* lineItem=dynamic_cast<QGraphicsLineItem*>(scene->items().front());
                Node* startNode=this->btnGrabNode(lineItem,startPos);
                Node* endNode=this->btnGrabNode(lineItem,endPos);
                scene->removeItem(lineItem);
                auto line=this->linkBetweenTwoNode(startNode,endNode);
                if(line==nullptr){
                    buttonType=Qt::NoButton;
                    return QWidget::eventFilter(watched,event);
                }
                buttonType=Qt::NoButton;
            }
            if(e->button()==Qt::LeftButton){
                QPainterPath selecPath;
                selecPath.addRect(selectionArea.rect());
                selecPath.translate(startPos);
                scene->setSelectionArea(selecPath);
                this->setSelectedNodeColor();
//                while(scene->items().contains(&selectionArea))
//                    scene->removeItem(&selectionArea);
//                selectionArea.setRect(QRectF{});
            }
            if(e->button()==Qt::LeftButton && startPos==endPos){
                while(scene->items().contains(&selectionArea))
                    scene->removeItem(&selectionArea);
                selectionArea.setRect(QRectF{});
                this->selectedNodes.clear();
                this->selectedLines.clear();
            }
            if(e->button()==Qt::LeftButton && !selectionArea.contains(e->scenePos())){
                while(scene->items().contains(&selectionArea))
                    scene->removeItem(&selectionArea);
                selectionArea.setRect(QRectF{});
                this->selectedNodes.clear();
                this->selectedLines.clear();
            }
        }
        if(event->type()==QEvent::KeyRelease){
            auto e=dynamic_cast<QKeyEvent*>(event);
            if(e->keyCombination()==QKeyCombination(Qt::ControlModifier,Qt::Key_Z)){
                undoStack->canUndo() ? undoStack->undo() : void();
            }
        }
    }

    return QWidget::eventFilter(watched,event);
}

const std::vector<LinkLine *> Graph::getLinkLineVec() const
{
    return linkLineVec;
}

std::vector<LinkLine *> Graph::getLinkLineVec()
{
    return linkLineVec;
}

QUndoStack * const Graph::getUndoStack()
{
    return this->undoStack;
}

void Graph::removeLinkLine(LinkLine *line)
{
    if(!line)   return;

    scene->removeItem(line);
    auto iter=std::find(linkLineVec.begin(),linkLineVec.end(),line);
    if(iter!=linkLineVec.end())
        linkLineVec.erase(iter);
}

void Graph::removeNode(Node *node)
{
    if(!node) return;

    scene->removeItem(node);

    std::stack<Node*> nodeStack1;
    Node* fnode=nullptr;
    while(fnode!=node && !nodeQue.empty()){
        fnode=nodeQue.front();
        nodeQue.pop();

        nodeStack1.push(fnode);
    }
    if(fnode==node){
        //nodeQue.pop();
        delete node;
        node=nullptr;
    }

    std::stack<Node*> nodeStack2;
    while(!nodeQue.empty()){
        nodeStack2.push(nodeQue.front());
        nodeQue.pop();
    }

    while(!nodeStack1.empty()){
        if(nodeStack1.top()==fnode){
            nodeStack1.pop();
            break;
        }
        nodeQue.push(nodeStack1.top());
        nodeStack1.pop();
    }
    while(!nodeStack2.empty()){
        nodeQue.push(nodeStack2.top());
        nodeStack2.pop();
    }
}

void Graph::setSelectedNodeColor()
{
    QList<QGraphicsItem*> nodeList=scene->selectedItems();
    for(auto& node:nodeList){
        auto n=dynamic_cast<Node*>(node);
        if(n){
            selectedNodes.push_back(n);
            continue;
        }
        auto l=dynamic_cast<LinkLine*>(node);
        if(l){
            selectedLines.push_back(l);
        }
    }

    for(auto& n:selectedNodes){
        n->setNodeStatus(true);
    }
}

Node *Graph::btnGrabNode(QGraphicsItem* other,const QPointF& pos)
{
    auto items=other->collidingItems();
    for(auto& i:items){
        if(i->contains(i->mapFromScene(pos))){
            return dynamic_cast<Node*>(i);
        }
    }

    return nullptr;
}

const LinkLine *Graph::linkBetweenTwoNode(Node *start, Node *end)
{
    if(start!=nullptr && end!=nullptr){
        auto hasLinkType=lxj::judgeLinkTypeBetweenTwoNode(start,end);
        if(hasLinkType!=LinkLine::SetLeftOrRight::NoLeftOrRight)
            return nullptr;
        if(start->getNode()->left!=nullptr&&start->getNode()->right!=nullptr)
            return nullptr;
        if(!end->collidingItems().empty())  return nullptr;

        auto linkType=hasLinkType;
        if(start->getNode()->left==nullptr) linkType=LinkLine::SetLeftOrRight::Left;
        if(start->getNode()->right==nullptr) linkType=LinkLine::SetLeftOrRight::Right;
        LinkLine* line=new LinkLine(start,end,linkType);
        scene->addItem(line);
        linkLineVec.push_back(line);
        start->setZValue(1);
        linkLineVec.back()->setZValue(0);

        start->addLinkline(line);
        end->addLinkline(line);

        LinkLineUndoCommand* undoLink=new LinkLineUndoCommand(this,line,"撤销连接");
        undoStack->push(undoLink);

        return line;
    }
    return nullptr;
}

const Node *Graph::getNodeAtPos(const QPointF &pos)
{
    if(nodeQue.empty()) return nullptr;
    std::queue<Node*> nQue=nodeQue;
    Node* n=nullptr;
    double len=std::numeric_limits<double>::max();
    while(!nQue.empty()){
        auto temNode=nQue.front();
        double distance=(pos-(temNode->mapToScene(temNode->boundingRect().center()))).manhattanLength();
        double l=std::min(len,distance);
        if(l<len){
            len=l;
            n=temNode;
        }
        nQue.pop();
    }

    return n->contains(n->mapFromScene(pos)) ? n:nullptr;
}

