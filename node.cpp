#include "node.h"
#include "linkline.h"
#include "graph.h"
#include "myundocommand.h"

#include<QGraphicsScene>
#include<QFont>
#include<QPainter>
#include<QGraphicsSceneMouseEvent>
#include<QVector2D>

Node::Node(const short value):node(new lxj::oneNode(value)),buttonEvent(Qt::NoButton),m_showBorder(false)
{
    borderLen=this->getNumDigit(value)*QFont().pointSize()+10;
    this->setFlag(QGraphicsItem::ItemIsSelectable);

}

Node::~Node()
{
    if(node){
        delete node;
        node=nullptr;
    }
//    if(borderStatus){
//        delete borderStatus;
//        borderStatus=nullptr;
//    }
    lineVec.clear();
}

const lxj::oneNode *Node::getNode()const
{
    return node;
}

lxj::oneNode *Node::getNode()
{
    return node;
}

const short Node::boderLength()
{
    return borderLen;
}

std::vector<LinkLine *>& Node::getLineList()
{
    return this->lineVec;
}

const std::vector<LinkLine *> &Node::getLineList() const
{
    return this->lineVec;
}

void Node::addLinkline(LinkLine *line)
{
    lineVec.push_back(line);
}

void Node::removeLinkLine(LinkLine *line)
{
    if(!line)   return;

    switch (line->getLOR()) {
    case LinkLine::SetLeftOrRight::Left:
        this->node->left=nullptr;
        break;
    case LinkLine::SetLeftOrRight::Right:
        this->node->right=nullptr;
    default:
        break;
    }
    auto iter=std::find(lineVec.begin(),lineVec.end(),line);
    if(iter!=lineVec.end())
        lineVec.erase(iter);      
}

void Node::setNodeStatus(const bool showBorder)
{
    this->m_showBorder=showBorder;
    this->update();
}

const unsigned short Node::getNumDigit(const short value) const
{
    double v=static_cast<double>(value);
    short i=0;
    while(v>=1){
        v/=10;
        ++i;
    }
    return i;
}

QRectF Node::boundingRect() const
{
    return QRectF(0,0,borderLen,borderLen);
}

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(155,120,123)));
    painter->drawEllipse(this->boundingRect());
    painter->setPen(Qt::black);
    if(node)
        painter->drawText(this->boundingRect(),Qt::AlignCenter,QString::number(node->value));

    if(this->m_showBorder){
        QPen pen;
        pen.setColor(QColor(76,110,87,150));
        pen.setWidth(2);
        pen.setStyle(Qt::DashLine);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(QRectF(0,0,borderLen,borderLen));
        //painter->drawLine(QLineF(0,0,borderLen,borderLen));
    }
}

QPainterPath Node::shape() const
{
    QPainterPath path;
    path.addEllipse(this->boundingRect());
    return path;
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    buttonEvent=event->button();
    postPos=this->scenePos();
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    buttonEvent=Qt::NoButton;
    //qDebug()<<(this->scenePos()==postPos);
    if(event->button()==Qt::LeftButton && this->scenePos()!=postPos){
        QObject* parentObject=this->scene()->parent();
        Graph* parentGraph=dynamic_cast<Graph*>(parentObject);
        if(parentGraph)
            parentGraph->getUndoStack()->push(new NodeMoveUndoCommand(this,postPos,"撤销位置"));
    }
}

void Node::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->buttons()==Qt::LeftButton){
        double x=0;
        double y=0;
        double mx=event->scenePos().x();
        double my=event->scenePos().y();

        if(mx<0 || (mx+borderLen)>this->scene()->width()){
            x=mx<0 ? 0:this->scene()->width()-borderLen;
        }
        else{
            x=mx-borderLen/2;
        }

        if(my<0 || (my+borderLen)>this->scene()->height()-borderLen){
            y=my<0 ? 0:this->scene()->height()-borderLen;
        }
        else{
            y=my-borderLen/2;
        }

        this->setPos(x,y);
    }

}

