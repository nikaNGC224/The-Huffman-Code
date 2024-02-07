#ifndef NODE_HPP
#define NODE_HPP

#include <iostream>
#include <memory>

namespace rassokhina
{
  class Node
  {
  public:
    using node_t = std::shared_ptr< Node >;
    node_t left_{ 0 };
    node_t right_{ 0 };
    node_t parent_{ 0 };

    Node() = default;
    Node(unsigned char symbol_, int frequency);
    Node(const std::string& name, int frequency);

    int getFrequency() const;
    void setFrequency(int f);

    std::string getCode() const;
    void setCode(const std::string& c);

    std::string getName() const;
    unsigned char getSymbol() const;

  private:
    std::string name_{ "" };
    unsigned char symbol_{ 0 };
    int frequency_{ 0 };
    std::string code_string_{ "" };
  };

  class LowestPriority
  {
  public:
    bool operator()(const Node::node_t& left, const Node::node_t& right) const
    {
      return left->getFrequency() > right->getFrequency();
    }
  };
}

#endif
