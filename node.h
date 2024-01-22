#ifndef NODE_H
#define NODE_H
#include "OneNode.hpp"

#include<vector>

#include <QGraphicsItem>

class LinkLine;
class Node : public QGraphicsItem
{
    //Q_OBJECT
public:
    Node(const short value);
    ~Node() override;

public:
    const lxj::oneNode* getNode() const;
    lxj::oneNode* getNode();
    const short boderLength();

    //获取连接线列表
    std::vector<LinkLine *>& getLineList();
    const std::vector<LinkLine *>& getLineList() const;

    //为此节点添加连接线
    void addLinkline(LinkLine* line);

    //删除此节点的某条连接线(此操作只是解除 节点 与 线 的连接关系，真正删除连接线所持有的资源只能由主界面进行操作)
    void removeLinkLine(LinkLine* line);

    //改变节点边框显示形态
    void setNodeStatus(const bool showBorder=false);

    // QGraphicsItem interface
public:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;

signals:

    // QGraphicsItem interface
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    lxj::oneNode* node;
    short borderLen;
    QPointF postPos;

    bool m_showBorder;


    //此节点上拥有的连接线集合
    std::vector<LinkLine*> lineVec;

    Qt::MouseButton buttonEvent;

private:
    const unsigned short getNumDigit(const short value) const;

};

#endif // NODE_H
