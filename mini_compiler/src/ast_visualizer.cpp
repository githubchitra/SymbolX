#include "ast_visualizer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <functional>

ASTVisualizer::ASTVisualizer(std::shared_ptr<ASTNode> root) : root(root) {}

std::string ASTVisualizer::toDOT() {
    if (!root) {
        return "digraph AST { \"Root\" [label=\"Empty AST\"]; }";
    }
    
    std::ostringstream oss;
    oss << "digraph AST {\n";
    oss << "  rankdir=TB;\n";
    oss << "  node [shape=box, style=filled, fontname=\"Courier\"];\n";
    oss << "  edge [fontname=\"Courier\"];\n\n";
    
    oss << generateDOT(root);
    
    oss << "}\n";
    return oss.str();
}

std::string ASTVisualizer::generateDOT(std::shared_ptr<ASTNode> node) {
    if (!node) return "";
    
    std::ostringstream oss;
    std::string nodeId = generateNodeID(node);
    std::string label = getNodeLabel(node);
    std::string color = getNodeTypeColor(node->getType());
    
    oss << "  \"" << nodeId << "\" [label=\"" << label << "\", fillcolor=\"" << color << "\"];\n";
    
    for (const auto& child : node->getChildren()) {
        std::string childId = generateNodeID(child);
        oss << "  \"" << nodeId << "\" -> \"" << childId << "\";\n";
        oss << generateDOT(child);
    }
    
    return oss.str();
}

std::string ASTVisualizer::toTextTree() {
    if (!root) {
        return "Empty AST\n";
    }
    
    return generateTextTree(root);
}

std::string ASTVisualizer::generateTextTree(std::shared_ptr<ASTNode> node, int depth) {
    if (!node) return "";
    
    std::ostringstream oss;
    std::string indent(depth * 2, ' ');
    
    oss << indent << "├─ " << node->toString();
    
    if (!node->getChildren().empty()) {
        oss << "\n";
        for (size_t i = 0; i < node->getChildren().size(); ++i) {
            if (i == node->getChildren().size() - 1) {
                std::string childStr = generateTextTree(node->getChildren()[i], depth + 1);
                // Replace the first ├─ with └─ for the last child
                if (!childStr.empty()) {
                    childStr.replace(0, 4, "└─ ");
                }
                oss << childStr;
            } else {
                oss << generateTextTree(node->getChildren()[i], depth + 1);
            }
        }
    }
    
    return oss.str();
}

std::string ASTVisualizer::toJSON() {
    if (!root) {
        return "{\"error\": \"Empty AST\"}";
    }
    
    return generateJSON(root);
}

std::string ASTVisualizer::generateJSON(std::shared_ptr<ASTNode> node) {
    if (!node) return "null";
    
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"type\": \"" << ASTNode::nodeTypeToString(node->getType()) << "\",\n";
    oss << "  \"value\": \"" << escapeString(node->getValue()) << "\",\n";
    oss << "  \"dataType\": \"" << ASTNode::dataTypeToString(node->getDataType()) << "\",\n";
    oss << "  \"line\": " << node->getLine() << ",\n";
    oss << "  \"column\": " << node->getColumn() << ",\n";
    oss << "  \"children\": [";
    
    const auto& children = node->getChildren();
    for (size_t i = 0; i < children.size(); ++i) {
        oss << generateJSON(children[i]);
        if (i < children.size() - 1) {
            oss << ",";
        }
    }
    
    oss << "]\n";
    oss << "}";
    
    return oss.str();
}

std::string ASTVisualizer::toXML() {
    if (!root) {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<ast error=\"Empty AST\"/>\n";
    }
    
    std::ostringstream oss;
    oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    oss << generateXML(root);
    return oss.str();
}

std::string ASTVisualizer::generateXML(std::shared_ptr<ASTNode> node, int depth) {
    if (!node) return "";
    
    std::ostringstream oss;
    std::string indent(depth * 2, ' ');
    
    oss << indent << "<node>\n";
    oss << indent << "  <type>" << ASTNode::nodeTypeToString(node->getType()) << "</type>\n";
    oss << indent << "  <value>" << escapeString(node->getValue()) << "</value>\n";
    oss << indent << "  <dataType>" << ASTNode::dataTypeToString(node->getDataType()) << "</dataType>\n";
    oss << indent << "  <line>" << node->getLine() << "</line>\n";
    oss << indent << "  <column>" << node->getColumn() << "</column>\n";
    
    if (!node->getChildren().empty()) {
        oss << indent << "  <children>\n";
        for (const auto& child : node->getChildren()) {
            oss << generateXML(child, depth + 2);
        }
        oss << indent << "  </children>\n";
    }
    
    oss << indent << "</node>\n";
    
    return oss.str();
}

std::string ASTVisualizer::toMermaid() {
    if (!root) {
        return "graph TD\n    A[Empty AST]";
    }
    
    std::ostringstream oss;
    oss << "graph TD\n";
    
    std::function<void(std::shared_ptr<ASTNode>)> generateMermaidNode;
    generateMermaidNode = [&](std::shared_ptr<ASTNode> node) {
        if (!node) return;
        
        std::string nodeId = generateNodeID(node);
        std::string label = getNodeLabel(node);
        
        // Clean up the label for Mermaid
        std::replace(label.begin(), label.end(), '"', '\'');
        
        oss << "    " << nodeId << "[\"" << label << "\"]\n";
        
        for (const auto& child : node->getChildren()) {
            std::string childId = generateNodeID(child);
            oss << "    " << nodeId << " --> " << childId << "\n";
            generateMermaidNode(child);
        }
    };
    
    generateMermaidNode(root);
    return oss.str();
}

std::string ASTVisualizer::escapeString(const std::string& str) {
    std::string escaped = str;
    size_t pos = 0;
    
    while ((pos = escaped.find("\"", pos)) != std::string::npos) {
        escaped.replace(pos, 1, "\\\"");
        pos += 2;
    }
    
    pos = 0;
    while ((pos = escaped.find("\n", pos)) != std::string::npos) {
        escaped.replace(pos, 1, "\\n");
        pos += 2;
    }
    
    return escaped;
}

std::string ASTVisualizer::getNodeLabel(std::shared_ptr<ASTNode> node) {
    if (!node) return "";
    
    std::ostringstream oss;
    oss << ASTNode::nodeTypeToString(node->getType());
    
    if (!node->getValue().empty()) {
        oss << "\\n'" << node->getValue() << "'";
    }
    
    if (node->getDataType() != DataType::UNKNOWN) {
        oss << "\\n:" << ASTNode::dataTypeToString(node->getDataType());
    }
    
    oss << "\\n(L" << node->getLine() << ":C" << node->getColumn() << ")";
    
    return oss.str();
}

std::string ASTVisualizer::getNodeTypeColor(NodeType type) {
    switch (type) {
        case NodeType::PROGRAM: return "lightblue";
        case NodeType::FUNCTION_DECLARATION: return "lightgreen";
        case NodeType::FUNCTION_DEFINITION: return "green";
        case NodeType::VARIABLE_DECLARATION: return "lightyellow";
        case NodeType::EXPRESSION_STATEMENT: return "lightcoral";
        case NodeType::IF_STATEMENT: return "lightpink";
        case NodeType::WHILE_STATEMENT: return "lightsalmon";
        case NodeType::FOR_STATEMENT: return "lightgoldenrodyellow";
        case NodeType::RETURN_STATEMENT: return "plum";
        case NodeType::BINARY_EXPRESSION: return "lightsteelblue";
        case NodeType::ASSIGNMENT_EXPRESSION: return "lightcyan";
        case NodeType::FUNCTION_CALL: return "lavender";
        case NodeType::IDENTIFIER_EXPRESSION: return "thistle";
        case NodeType::LITERAL_EXPRESSION: return "mistyrose";
        default: return "white";
    }
}

std::string ASTVisualizer::generateNodeID(std::shared_ptr<ASTNode> node) {
    if (!node) return "null";
    
    std::ostringstream oss;
    oss << "node_" << node.get() << "_" << node->getType();
    return oss.str();
}

void ASTVisualizer::printDOT() {
    std::cout << toDOT() << std::endl;
}

void ASTVisualizer::printTextTree() {
    std::cout << toTextTree() << std::endl;
}

void ASTVisualizer::printJSON() {
    std::cout << toJSON() << std::endl;
}

void ASTVisualizer::printXML() {
    std::cout << toXML() << std::endl;
}

void ASTVisualizer::printMermaid() {
    std::cout << toMermaid() << std::endl;
}

void ASTVisualizer::printStatistics() {
    if (!root) {
        std::cout << "AST Statistics: Empty tree\n";
        return;
    }
    
    std::unordered_map<NodeType, int> nodeCounts;
    int maxDepth = 0;
    calculateStatistics(root, nodeCounts, maxDepth);
    
    std::cout << "=== AST Statistics ===\n";
    std::cout << "Total nodes: " << nodeCounts.size() << "\n";
    std::cout << "Maximum depth: " << maxDepth << "\n\n";
    
    std::cout << "Node types:\n";
    for (const auto& pair : nodeCounts) {
        std::cout << "  " << ASTNode::nodeTypeToString(pair.first) << ": " << pair.second << "\n";
    }
}

bool ASTVisualizer::saveDOT(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << toDOT();
    file.close();
    return true;
}

bool ASTVisualizer::saveTextTree(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << toTextTree();
    file.close();
    return true;
}

bool ASTVisualizer::saveJSON(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << toJSON();
    file.close();
    return true;
}

bool ASTVisualizer::saveXML(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << toXML();
    file.close();
    return true;
}

bool ASTVisualizer::saveMermaid(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << toMermaid();
    file.close();
    return true;
}

void ASTVisualizer::setRoot(std::shared_ptr<ASTNode> newRoot) {
    root = newRoot;
}

void ASTVisualizer::calculateStatistics(std::shared_ptr<ASTNode> node, 
                                      std::unordered_map<NodeType, int>& nodeCounts,
                                      int& maxDepth, int currentDepth) {
    if (!node) return;
    
    nodeCounts[node->getType()]++;
    maxDepth = std::max(maxDepth, currentDepth);
    
    for (const auto& child : node->getChildren()) {
        calculateStatistics(child, nodeCounts, maxDepth, currentDepth + 1);
    }
}

std::string ASTVisualizer::getVisualizationInfo() {
    return "AST Visualizer v1.0\n"
           "Supported formats:\n"
           "  - DOT (Graphviz)\n"
           "  - Text Tree\n"
           "  - JSON\n"
           "  - XML\n"
           "  - Mermaid\n"
           "Features:\n"
           "  - Color-coded node types\n"
           "  - Detailed node information\n"
           "  - Statistics generation\n"
           "  - Multiple output formats";
}
