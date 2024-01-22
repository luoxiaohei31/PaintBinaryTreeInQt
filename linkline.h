#ifndef LINKLINE_H
#define LINKLINE_H

#include "node.h"

#include <thread>
#include<mutex>
#include<condition_variable>
#include<queue>

#include <QGraphicsLineItem>
#include <QUndoCommand>

class LinkLine : public QGraphicsLineItem
{
    //friend class LinkLineUndoCommand;

public:
    enum struct SetLeftOrRight:unsigned short{Left=0,NoLeftOrRight=10,Right=20};

    LinkLine();
    LinkLine(Node* s,Node* e,SetLeftOrRight LOR=SetLeftOrRight::Left);
    ~LinkLine();

    //返回此连接线的类型(是某个节点的左连接线，还是右连接线)
    const SetLeftOrRight& getLOR();

    //返回此连接线end节点的位置
    const QPointF getEndPos()const;

    //返回此连接线的QLineF
    const QLineF getLine();

    //返回头节点
    Node* const startNode();

    //返回尾节点
    Node* const endNode();

    //修改
    void updateLine(const QLineF &line);
    void updateSelf();

    //设置连接线类型(若此连接线(类型为left)的头节点已有另一条连接线(类型为right)，则设置失败)
    void setLOR(const LinkLine::SetLeftOrRight lor);

private:
    Node* start;
    Node* end;
    SetLeftOrRight m_lor;

    std::thread calcuEndPos;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::queue<QPointF> pointQue;
    short pixelNeedToUpdate;

private:
    //给pointQue添加point
    void addPointToQue();
};

namespace lxj {
const LinkLine::SetLeftOrRight judgeLinkTypeBetweenTwoNode(const Node* sNode,const Node* eNode);
}

#endif // LINKLINE_H
