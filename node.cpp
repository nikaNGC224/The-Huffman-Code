#include "node.hpp"
#include <iostream>

rassokhina::Node::Node(unsigned char symbol,
    int frequency):
  symbol_(symbol),
  frequency_(frequency)
{}

rassokhina::Node::Node(const std::string& name,
    int frequency):
  name_(name),
  frequency_(frequency)
{}

int rassokhina::Node::getFrequency() const
{
  return frequency_;
}

void rassokhina::Node::setFrequency(int f)
{
  frequency_ = f;
}

std::string rassokhina::Node::getCode() const
{
  return code_string_;
}

void rassokhina::Node::setCode(const std::string& c)
{
  code_string_ = c;
}

std::string rassokhina::Node::getName() const
{
  if (symbol_ == 0)
  {
    return name_;
  }
  else
  {
    if (symbol_ == '\n')
    {
      return "\\n";
    }
    return std::string(1, static_cast< char >(symbol_));
  }
}

unsigned char rassokhina::Node::getSymbol() const
{
  return symbol_;
}
