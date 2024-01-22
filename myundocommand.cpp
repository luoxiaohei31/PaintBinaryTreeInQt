#include "myundocommand.h"
#include "graph.h"
#include "linkline.h"
//#include "node.h"

LinkLineUndoCommand::LinkLineUndoCommand(Graph *parent, LinkLine* undoLine, const QString &undoInfo)
    :m_parent(parent),m_undoLine(undoLine)
{
    this->setText(undoInfo);
}

void LinkLineUndoCommand::undo()
{
    if(m_undoLine){
        m_undoLine->startNode()->removeLinkLine(m_undoLine);
        m_undoLine->endNode()->removeLinkLine(m_undoLine);
        m_parent->removeLinkLine(m_undoLine);
        delete m_undoLine;
        m_undoLine=nullptr;
    }
}

NodeMoveUndoCommand::NodeMoveUndoCommand(Node *node, const QPointF &p, const QString &undoInfo)
    :m_node(node),m_pos(p)
{
    this->setText(undoInfo);
}

void NodeMoveUndoCommand::undo()
{
    if(!m_node) return;

    m_node->setPos(m_pos);
    for(auto& l:m_node->getLineList()){
        l->updateSelf();
    }
}
