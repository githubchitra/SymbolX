#ifndef AST_VISUALIZER_H
#define AST_VISUALIZER_H

#include "ast_node.h"
#include <string>
#include <vector>
#include <memory>

class ASTVisualizer {
private:
    std::shared_ptr<ASTNode> root;
    
    // Visualization methods
    std::string generateDOT(std::shared_ptr<ASTNode> node);
    std::string generateTextTree(std::shared_ptr<ASTNode> node, int depth = 0);
    std::string generateJSON(std::shared_ptr<ASTNode> node);
    std::string generateXML(std::shared_ptr<ASTNode> node, int depth = 0);
    
    // Helper methods
    std::string escapeString(const std::string& str);
    std::string getNodeLabel(std::shared_ptr<ASTNode> node);
    std::string getNodeTypeColor(NodeType type);
    std::string generateNodeID(std::shared_ptr<ASTNode> node);
    
    // Statistics
    void calculateStatistics(std::shared_ptr<ASTNode> node, 
                           std::unordered_map<NodeType, int>& nodeCounts,
                           int& maxDepth, int currentDepth = 0);

public:
    explicit ASTVisualizer(std::shared_ptr<ASTNode> root);
    
    // Output formats
    std::string toDOT();
    std::string toTextTree();
    std::string toJSON();
    std::string toXML();
    std::string toMermaid();
    
    // Display methods
    void printDOT();
    void printTextTree();
    void printJSON();
    void printXML();
    void printMermaid();
    void printStatistics();
    
    // File output
    bool saveDOT(const std::string& filename);
    bool saveTextTree(const std::string& filename);
    bool saveJSON(const std::string& filename);
    bool saveXML(const std::string& filename);
    bool saveMermaid(const std::string& filename);
    
    // Utility methods
    void setRoot(std::shared_ptr<ASTNode> newRoot);
    std::shared_ptr<ASTNode> getRoot() const { return root; }
    
    // Static utility methods
    static std::string getVisualizationInfo();
};

#endif // AST_VISUALIZER_H
