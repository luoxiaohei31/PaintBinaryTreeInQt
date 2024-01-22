#ifndef GRAPH_H
#define GRAPH_H

#include <QWidget>
#include<QUndoStack>

#include<queue>

QT_BEGIN_NAMESPACE
namespace Ui { class Graph; }
QT_END_NAMESPACE


class Node;
class LinkLine;
class QGraphicsScene;
class QGraphicsItem;
class QGraphicsSceneMouseEvent;
class Graph : public QWidget
{
    Q_OBJECT

public:
    Graph(QWidget *parent = nullptr);
    ~Graph();

    //获取输入栏的数字
    void getNums();

    //找出other碰撞的其它Item中 所在位置为pos的Item
    Node* btnGrabNode(QGraphicsItem* other, const QPointF& pos);

    //划出两个Node之间的连接线，并返回连接线的指针
    const LinkLine* linkBetweenTwoNode(Node* start,Node* end);

    //返回位于位置pos的Node，若没有则返回nullptr
    const Node* getNodeAtPos(const QPointF& pos);

    //返回连接线列表
    const std::vector<LinkLine *> getLinkLineVec() const;
    std::vector<LinkLine *> getLinkLineVec();

    //返回“撤销栈”
    QUndoStack* const getUndoStack();

    /********************以下为修改操作********************/

    //删除某条连接线
    void removeLinkLine(LinkLine* line);

    //删除某个节点
    void removeNode(Node* node);

    //设置被选中的node的显示状态
    void setSelectedNodeColor();

public slots:
    void generateNodes();
    void clear();

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:

    Ui::Graph *ui;
    QGraphicsScene* scene;

    std::queue<int> numsQue;
    std::queue<Node*> nodeQue;
    std::vector<LinkLine*> linkLineVec;

    std::vector<Node*> selectedNodes;
    std::vector<LinkLine*> selectedLines;

    //撤销操作
    QUndoStack* undoStack;
};
#endif // GRAPH_H
