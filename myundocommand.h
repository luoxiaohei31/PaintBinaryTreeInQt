#ifndef MYUNDOCOMMAND_H
#define MYUNDOCOMMAND_H

#include <QUndoCommand>
#include<QPointF>

class Graph;
class LinkLine;
class Node;

class LinkLineUndoCommand: public QUndoCommand{
public:
    LinkLineUndoCommand(Graph* parent=nullptr,LinkLine* undoLine=nullptr, const QString& undoInfo="");
    void undo() override;

private:
    Graph* m_parent;
    LinkLine* m_undoLine;
};


class NodeMoveUndoCommand: public QUndoCommand{
public:
    NodeMoveUndoCommand(Node* node=nullptr,const QPointF& p=QPointF(), const QString& undoInfo="");
    void undo() override;

private:
    Node* m_node;
    QPointF m_pos;
};

#endif // MYUNDOCOMMAND_H
