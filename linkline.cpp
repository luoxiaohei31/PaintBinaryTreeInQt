#include "linkline.h"

#include<iostream>

#include<QPainter>
#include<QPen>
#include<QBrush>

LinkLine::LinkLine():start(nullptr),end(nullptr),m_lor(SetLeftOrRight::NoLeftOrRight),QGraphicsLineItem(QLineF())
{

}

LinkLine::LinkLine(Node *s, Node *e,SetLeftOrRight LOR):start(s),end(e),m_lor(LOR)
{
    const QLineF line=this->getLine();

    this->setPen(QPen(QBrush(QColor(27,75,20)),3));
    this->setLine(line);

    if(m_lor==SetLeftOrRight::Left) start->getNode()->left=end->getNode();
    if(m_lor==SetLeftOrRight::Right) start->getNode()->right=end->getNode();

}

LinkLine::~LinkLine()
{
    if(calcuEndPos.joinable())  calcuEndPos.join();
}

const LinkLine::SetLeftOrRight &LinkLine::getLOR()
{
    return m_lor;
}

const QPointF LinkLine::getEndPos() const
{
    if(start==nullptr || end==nullptr)  return QPointF();
    auto sPos=start->boundingRect().center();
    auto ePos=end->boundingRect().center();
    QPointF sPoint=start->mapToScene(sPos);
    QPointF ePoint=end->mapToScene(ePos);

    const double endRadius=end->boderLength()/2;
    const double k=(ePoint.y()-sPoint.y())/(ePoint.x()-sPoint.x());
    const double d=ePoint.y()-k*ePoint.x();

    //由上得y=kx+d;
    //ePoint到所求点的距离为endRadius，可列公式：(ex-x)^2+(ey-y)^2=endRadius^2
    const double a=1+std::pow(k,2);
    const double b=2*(k*d-ePoint.x()-k*ePoint.y());
    const double c=std::pow(d-ePoint.y(),2)+(ePoint.x()+endRadius)*(ePoint.x()-endRadius);

    const double x1=(-b+std::sqrt(std::pow(b,2)-4*a*c))/(2*a);
    const double x2=(-b-std::sqrt(std::pow(b,2)-4*a*c))/(2*a);

    double x,y;
    if(ePoint.x()>sPoint.x()){
        x=x1<ePoint.x() ? x1:x2;
    }
    if(ePoint.x()<sPoint.x()){
        x=x1>ePoint.x() ? x1:x2;
    }
    y=k*x+d;

    return QPointF(x,y);
}

const QLineF LinkLine::getLine()
{
    QPointF spos=start->mapToScene(start->boundingRect().center());
    //QPointF epos=end->mapToScene(end->boundingRect().center());
    QPointF epos=this->getEndPos();
    //auto ePromise=std::async(std::launch::async,&LinkLine::getEndPos,this);

    return QLineF(spos,epos);
}

Node * const LinkLine::startNode()
{
    return start;
}

Node * const LinkLine::endNode()
{
    return end;
}

void LinkLine::updateLine(const QLineF &lineVec)
{
    this->setLine(lineVec);
}

void LinkLine::updateSelf()
{
    //this->updateLine(this->getLine());

    if(calcuEndPos.joinable())  calcuEndPos.join();
    calcuEndPos=std::thread(&LinkLine::addPointToQue,this);

    std::unique_lock<std::mutex> qm(queueMutex);
    cv.wait(qm,[this]{return !pointQue.empty();});

    QPointF spos=start->mapToScene(start->boundingRect().center());
    while(!pointQue.empty()){
        QPointF epos=pointQue.front();
        pointQue.pop();

        this->updateLine(QLineF(spos,epos));
    }
    cv.notify_all();
}

void LinkLine::setLOR(const SetLeftOrRight lor)
{
    if(lor==m_lor)  return;

    if(lor==SetLeftOrRight::Left){
        this->start->getNode()->left=this->end->getNode();
        this->start->getNode()->right=nullptr;
        m_lor=lor;
    }
    if(lor==SetLeftOrRight::Right){
        this->start->getNode()->right=this->end->getNode();
        this->start->getNode()->left=nullptr;
        m_lor=lor;
    }
}

void LinkLine::addPointToQue()
{
    std::unique_lock<std::mutex> qm(queueMutex);
    cv.wait(qm,[=]{return this->pointQue.empty();});

    auto pos=this->getEndPos();
    pointQue.push(pos);

    cv.notify_all();
}

const LinkLine::SetLeftOrRight lxj::judgeLinkTypeBetweenTwoNode(const Node *sNode, const Node *eNode)
{
    if(sNode==nullptr || eNode==nullptr)    return LinkLine::SetLeftOrRight::NoLeftOrRight;
    if(sNode->getNode()->left==eNode->getNode())    return LinkLine::SetLeftOrRight::Left;
    if(sNode->getNode()->right==eNode->getNode())   return LinkLine::SetLeftOrRight::Right;

    return LinkLine::SetLeftOrRight::NoLeftOrRight;
}
