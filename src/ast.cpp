#include "ast.hpp"

ASTNode::ASTNode(Token t, NodeType nt, std::vector<Ptr>&& c) 
    : token(t), type(nt), children(std::move(c)) {
    for (auto& child : children) {
        if (child) {
            child->parent = this;
        }
    }
}

void ASTTree::setRoot(ASTNode::Ptr node) {
    root = std::move(node);
}

ASTNode* ASTTree::getRoot() const {
    return root.get();
}

void ASTTree::clear() {
    root.reset();
}

bool ASTTree::isEmpty() const {
    return !root;
}

void ASTTree::traversePreOrder(ASTNode* node, const std::function<void(ASTNode*)>& func) {
    if (!node) return;
    func(node);
    for (auto& child : node->children) {
        traversePreOrder(child.get(), func);
    }
}

void ASTTree::traverseLevelOrder(const std::function<void(ASTNode*)>& func) {
    if (!root) return;
    std::queue<ASTNode*> queue;
    queue.push(root.get());
    
    while (!queue.empty()) {
        ASTNode* node = queue.front();
        queue.pop();
        func(node);
        
        for (auto& child : node->children) {
            queue.push(child.get());
        }
    }
}
