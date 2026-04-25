#include "visitor.h"
#include "node.h"

void Visitor::dispatch(Node* node) {
    auto it = table_.find(typeid(*node));
    if (it != table_.end()) {
        it->second(node);
    }
}
