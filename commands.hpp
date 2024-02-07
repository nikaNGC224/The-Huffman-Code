#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "node.hpp"
#include <iosfwd>
#include <map>
#include <queue>
#include <vector>
#include <string>

namespace rassokhina
{
  class Command
  {
  public:
    using priotity_queue_t = std::priority_queue< rassokhina::Node::node_t, std::vector< rassokhina::Node::node_t >,
      rassokhina::LowestPriority >;
    using read_data_t = std::map< std::string, std::string >;
    using code_data_t = std::map< std::string, std::vector< std::string > >;
    Command() = default;
    void work(std::istream& in, std::ostream& out);
    static void help(std::ostream& out);
    static void encode(std::string& line, read_data_t& readData, code_data_t& codeData);
    static void decode(std::string& line, read_data_t& readData, code_data_t& codeData);
    static void list(std::ostream& out, std::string& line, read_data_t& readData);
    static void read(std::istream& in, std::ostream& out, std::string& line, read_data_t& readData);
    static void flush(std::ostream& out, std::string& line, read_data_t& readData);
    static void equals(std::ostream& out, std::string& line, read_data_t& readData);
    static void concat(std::string& line, read_data_t& readData);
    static void merge(std::string& line, read_data_t& readData, code_data_t& codeData);
    static void inspect(std::ostream& out, std::string& line, read_data_t& readData, code_data_t& codeData);
    static void drop(std::string& line, read_data_t& readData, code_data_t& codeData);

  private:
    static void makeCode(rassokhina::Node::node_t& node, std::string str, std::vector< std::string >& codes);
    static void setQueue(std::vector< int > data, Command::priotity_queue_t& queue);
    static void buildTree(Command::priotity_queue_t& queue);
    static std::string textToCode(const std::string& text, const std::vector< std::string >& codes);
    static std::string codeToText(const std::string& text, const std::vector< std::string >& codes);
    static std::string doRead(std::istream& in, std::ostream& out);
    static std::string doRead(const std::string& fileName);
    static void doFlush(const std::string& text, std::ostream& out);
    static void doFlush(const std::string& text, const std::string& fileName);
  };
}

#endif
