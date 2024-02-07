#include "commands.hpp"
#include <iostream>
#include <fstream>
#include <queue>
#include <array>
#include <iterator>
#include <functional>

void rassokhina::Command::work(std::istream& in, std::ostream& out)
{
  std::string line;
  read_data_t readData;
  code_data_t codeData;
  std::map< std::string, std::function< void() > > list_(
    { { "help",    std::bind(rassokhina::Command::help,    std::ref(out)) },
      { "encode",  std::bind(rassokhina::Command::encode,  std::ref(line),
        std::ref(readData), std::ref(codeData)) },
      { "decode",  std::bind(rassokhina::Command::decode,
        std::ref(line), std::ref(readData), std::ref(codeData)) },
      { "list",    std::bind(rassokhina::Command::list,
        std::ref(out),  std::ref(line),     std::ref(readData)) },
      { "read",    std::bind(rassokhina::Command::read,
        std::ref(in),   std::ref(out),      std::ref(line), std::ref(readData)) },
      { "flush",   std::bind(rassokhina::Command::flush,
        std::ref(out),  std::ref(line),     std::ref(readData)) },
      { "equals",  std::bind(rassokhina::Command::equals,
        std::ref(out),  std::ref(line),     std::ref(readData)) },
      { "concat",  std::bind(rassokhina::Command::concat,
        std::ref(line), std::ref(readData)) },
      { "merge",   std::bind(rassokhina::Command::merge,
        std::ref(line), std::ref(readData), std::ref(codeData)) },
      { "inspect", std::bind(rassokhina::Command::inspect,
        std::ref(out),  std::ref(line),     std::ref(readData), std::ref(codeData)) },
      { "drop",    std::bind(rassokhina::Command::drop,
        std::ref(line), std::ref(readData), std::ref(codeData)) } } );

  std::string cmd;
  char space = ' ';

  while (true)
  {
    out << "cmd: ";
    std::getline(in, line);
    if (in.eof())
    {
      break;
    }
    if (line.empty())
    {
      continue;
    }
    if (line.find(space) == std::string::npos)
    {
      cmd = std::move(line);
    }
    else
    {
      std::copy(line.begin(), line.begin() + line.find(space), std::back_inserter(cmd));
      line.erase(line.begin(), line.begin() + line.find(space) + 1);
    }

    try
    {
      list_[cmd]();
    }
    catch (const std::exception& e)
    {
      out << e.what() << "\n";
    }
    cmd.clear();
  }
}

void rassokhina::Command::help(std::ostream& out)
{
  out <<"\nHuffman code - this program compresses text using the Huffman algorithm.\n"
            << "Supported commands:\n"
            << "-read    \"parameter\" - reads text as a single line from standard input into a variable "
            << "\"parameter\";\n"
            << "-read    \"parameter1\" \"parameter2\" - reads text from a file \"parameter2\" into a variable "
            << "\"parameter1\";\n"
            << "-flush   \"parameter\" - writes text \"parameter\" to standard output;\n"
            << "-flush   \"parameter1\" \"parameter2\" - outputs text \"parameter1\" to file \"parameter2\" "
            << "(with .txt);\n"
            << "-encode  \"parameter1\" \"parameter2\" - encodes the read text \"parameter1\" into a variable "
            << "\"parameter2\";\n"
            << "-decode  \"parameter1\" \"parameter2\" - decodes the encoded text \"parameter1\" into a variable "
            << "\"parameter2\";\n"
            << "-inspect \"parameter\" - displays information about the encoded text;\n"
            << "-equals  \"parameter1\" \"parameter2\" - compares the text of \"parameter1\" with \"parameter2\";\n"
            << "-merge   \"parameter1\" \"parameter2\" \"parameter3\" - turns duplicate data \"parameter1\" & "
            << "\"parameter2\";\n"
            << " into one variable \"parameter3\" if they are equal in data and encryption;\n"
            << "-concat  \"parameter1\" \"parameter2\" \"parameter3\" - combines \"parameter1\" & \"parameter2\" "
            << "into \"parameter3\";\n"
            << "-list - displays a list of all read texts;\n"
            << "-drop - deletes all read texts;\n"
            << "-drop    \"parameter\" - deletes data with name \"parameter\";\n"
            << "-ctrl+Z (Windows) or -ctrl+D (Linux) - exit the program.\n\n";
}

void rassokhina::Command::encode(std::string& line, read_data_t& readData, code_data_t& codeData)
{
  char space = ' ';
  if (line.find(space) == std::string::npos)
  {
    throw std::invalid_argument("encode: parameter missing");
  }
  std::string name;
  std::copy(line.begin(), line.begin() + line.find(space), std::back_inserter(name));
  line.erase(line.begin(), line.begin() + line.find(space) + 1);
  if (line.find(space) != std::string::npos)
  {
    throw std::invalid_argument("encode: too many parameters");
  }
  std::map< std::string, std::string >::const_iterator it = readData.find(name);
  if (it == readData.end())
  {
    throw std::logic_error("encode: this data is not read");
  }
  if (codeData.find(name) != codeData.end())
  {
    throw std::logic_error("encode: this data is already encoded");
  }
  if (it->second.empty())
  {
    throw std::logic_error("encode: this data has empty text");
  }
  std::vector< int > data(256, 0);
  for (std::size_t i = 0; i < it->second.size(); ++i)
  {
    data[static_cast< unsigned char >(it->second[i])]++;
  }
  Command::priotity_queue_t queue;
  setQueue(data, queue);
  buildTree(queue);

  std::vector< std::string > codes(256, "");
  rassokhina::Node::node_t root = queue.top();
  makeCode(root, "", codes);
  std::string textCode = textToCode(it->second, codes);
  if (readData.find(line) == readData.end())
  {
    readData.insert({ line, textCode });
  }
  else
  {
    readData[line] = textCode;
  }
  codeData.insert({ line, codes });
}

void rassokhina::Command::decode(std::string& line, read_data_t& readData, code_data_t& codeData)
{
  char space = ' ';
  if (line.find(space) == std::string::npos)
  {
    throw std::invalid_argument("decode: parameter missing");
  }
  std::string name;
  std::copy(line.begin(), line.begin() + line.find(space), std::back_inserter(name));
  line.erase(line.begin(), line.begin() + line.find(space) + 1);
  if (line.find(space) != std::string::npos)
  {
    throw std::invalid_argument("decode: too many parameters");
  }
  std::map< std::string, std::vector< std::string > >::const_iterator it = codeData.find(name);
  if (it == codeData.end())
  {
    throw std::logic_error("decode: this data is not encoded");
  }
  std::map< std::string, std::string >::const_iterator it1 = readData.find(name);
  std::string text = codeToText(it1->second, it->second);
  if (readData.find(line) == readData.end())
  {
    readData.insert({ line, text });
  }
  else
  {
    readData[line] = text;
  }
}

void rassokhina::Command::list(std::ostream& out, std::string& line, read_data_t& readData)
{
  if (!line.empty())
  {
    throw std::invalid_argument("list: too many parameters");
  }
  if (readData.empty())
  {
    throw std::logic_error("list: empty");
  }
  std::map< std::string, std::string >::const_iterator it = readData.begin();
  out << it->first;
  ++it;
  while (it != readData.end())
  {
    out << " " << it->first;
    ++it;
  }
  out << "\n";
}

void rassokhina::Command::read(std::istream& in, std::ostream& out, std::string& line, read_data_t& readData)
{
  char space = ' ';
  if (line.empty())
  {
    throw std::invalid_argument("read: parameter missing");
  }
  std::string name;
  if (line.find(space) == std::string::npos)
  {
    name = std::move(line);
  }
  else
  {
    std::copy(line.begin(), line.begin() + line.find(space), std::back_inserter(name));
    line.erase(line.begin(), line.begin() + line.find(space) + 1);
    if (line.find(space) != std::string::npos)
    {
      throw std::invalid_argument("read: too many parameters");
    }
  }
  if (readData.find(name) != readData.end())
  {
    throw std::logic_error("read: this data has already been read");
  }
  std::string text = (line.empty()) ? (doRead(in, out)) : (doRead(line));
  readData.insert({ name, text });
}

void rassokhina::Command::flush(std::ostream& out, std::string& line, read_data_t& readData)
{
  char space = ' ';
  if (line.empty())
  {
    throw std::invalid_argument("flush: parameter missing");
  }
  std::string name;
  if (line.find(space) == std::string::npos)
  {
    name = std::move(line);
  }
  else
  {
    std::copy(line.begin(), line.begin() + line.find(space), std::back_inserter(name));
    line.erase(line.begin(), line.begin() + line.find(space) + 1);
    if (line.find(space) != std::string::npos)
    {
      throw std::invalid_argument("flush: too many parameters");
    }
  }
  std::map< std::string, std::string >::iterator it = readData.find(name);
  if (it == readData.end())
  {
    throw std::logic_error("flush: this data is not read");
  }
  (line.empty()) ? (doFlush(it->second, out)) : (doFlush(it->second, line));
}

void rassokhina::Command::equals(std::ostream& out, std::string& line, read_data_t& readData)
{
  char space = ' ';
  if (line.find(space) == std::string::npos)
  {
    throw std::invalid_argument("equals: parameter missing");
  }
  std::array< std::pair< std::string, std::string >, 2 > data;
  std::copy(line.begin(), line.begin() + line.find(space), std::back_inserter(data[0].first));
  line.erase(line.begin(), line.begin() + line.find(space) + 1);
  if (line.find(space) != std::string::npos)
  {
    throw std::invalid_argument("equals: too many parameters");
  }
  data[1].first = std::move(line);
  for (std::size_t i = 0; i < 2; ++i)
  {
    std::map< std::string, std::string >::const_iterator it = readData.find(data[i].first);
    if (it == readData.end())
    {
      throw std::logic_error("equals: this data is not read");
    }
    data[i].second = it->second;
  }
  out << "these data are " << ((data[0].second == data[1].second) ? ("") : ("not ")) << "equal\n";
}

void rassokhina::Command::concat(std::string& line, read_data_t& readData)
{
  char space = ' ';
  std::array< std::string, 2 > data;
  for (std::size_t i = 0; i < 2; ++i)
  {
    if (line.find(space) == std::string::npos)
    {
      throw std::invalid_argument("concat: parameter missing");
    }
    std::copy(line.begin(), line.begin() + line.find(space), std::back_inserter(data[i]));
    line.erase(line.begin(), line.begin() + line.find(space) + 1);
  }
  if (line.find(space) != std::string::npos)
  {
    throw std::invalid_argument("concat: too many parameters");
  }
  std::string text;
  for (std::size_t i = 0; i < 2; ++i)
  {
    std::map< std::string, std::string >::const_iterator it = readData.find(data[i]);
    if (it == readData.end())
    {
      throw std::logic_error("concat: this data is not read");
    }
    std::copy(it->second.begin(), it->second.end(), std::back_inserter(text));
  }

  if ((data[0] == line) || (data[1] == line))
  {
    readData[line] = text;
  }
  else
  {
    readData.insert({ line, text });
  }
}

void rassokhina::Command::merge(std::string& line, read_data_t& readData, code_data_t& codeData)
{
  char space = ' ';
  std::array< std::string, 2 > data;

  for (std::size_t i = 0; i < 2; ++i)
  {
    if (line.find(space) == std::string::npos)
    {
      throw std::invalid_argument("merge: parameter missing");
    }
    std::copy(line.begin(), line.begin() + line.find(space), std::back_inserter(data[i]));
    line.erase(line.begin(), line.begin() + line.find(space) + 1);
  }
  if (line.find(space) != std::string::npos)
  {
    throw std::invalid_argument("merge: too many parameters");
  }
  for (std::size_t i = 0; i < 2; ++i)
  {
    if (readData.find(data[i]) == readData.end())
    {
      throw std::logic_error("merge: this data is not read");
    }
  }
  bool isEqualEncript = (codeData.find(data[0]) != codeData.end())
    == (codeData.find(data[1]) != codeData.end());
  if (!isEqualEncript)
  {
    throw std::logic_error("merge: these data have different encryption");
  }
  bool isEqual = readData[data[0]] == readData[data[1]];
  if (!isEqual)
  {
    throw std::logic_error("megre: these data have different text");
  }

  std::string text = readData[data[0]];
  readData.insert({ line, text });
  for (std::size_t i = 0; i < 2; ++i)
  {
    readData.erase(readData.find(data[i]));
  }
  bool isEncode = (codeData.find(data[0]) != codeData.end())
    && (codeData.find(data[1]) != codeData.end());
  if (isEncode)
  {
    std::vector< std::string > codes = codeData[data[0]];
    codeData.insert({ line, codes });
    for (std::size_t i = 0; i < 2; ++i)
    {
      codeData.erase(codeData.find(data[i]));
    }
  }
}

void rassokhina::Command::inspect(std::ostream& out, std::string& line, read_data_t& readData, code_data_t& codeData)
{
  if (line.empty())
  {
    throw std::invalid_argument("inspect: parameter missing");
  }
  if (line.find(' ') != std::string::npos)
  {
    throw std::invalid_argument("inspect: too many parameters");
  }
  if (readData.find(line) == readData.end())
  {
    throw std::logic_error("inspect: this data is not read");
  }
  if (codeData.find(line) == codeData.end())
  {
    throw std::logic_error("inspect: this data is not encoded");
  }

  out << "alphabet:      ";
  std::vector< std::string >::const_iterator it = codeData[line].begin();
  int i = 0;
  while ((*it == "") && (it != codeData[line].end()))
  {
    ++i;
    ++it;
  }
  out << "[" << static_cast<unsigned char>(i) << "] = " << *it;
  ++i;
  ++it;
  while (it != codeData[line].end())
  {
    if (*it != "")
    {
      out << " [" << static_cast<unsigned char>(i) << "] = " << *it;
    }
    ++i;
    ++it;
  }
  std::size_t textSize = codeToText(readData[line], codeData[line]).size();
  std::size_t newSize = readData[line].size();
  out << "\noriginal size: " << textSize * 8 << " bit\n"
      << "new size:      " << newSize << " bit\n"
      << "compression:   " << (((textSize * 8) - newSize) * 100) / (textSize * 8) << " %\n";
}

void rassokhina::Command::drop(std::string& line, read_data_t& readData, code_data_t& codeData)
{
  if (line.empty())
  {
    readData.clear();
    codeData.clear();
  }
  else
  {
    if (line.find(' ') != std::string::npos)
    {
      throw std::invalid_argument("drop: too many parameters");
    }
    if (readData.find(line) == readData.end())
    {
      throw std::logic_error("drop: this data is not read");
    }
    readData.erase(readData.find(line));
    if (codeData.find(line) != codeData.end())
    {
      codeData.erase(codeData.find(line));
    }
  }
}

void rassokhina::Command::makeCode(rassokhina::Node::node_t& node, std::string str, std::vector< std::string >& codes)
{
  if (node->left_ != nullptr)
  {
    makeCode(node->left_, str + "0", codes);
  }
  if (node->right_ != nullptr)
  {
    makeCode(node->right_, str + "1", codes);
  }
  if ((node->left_ == nullptr) && (node->right_ == nullptr))
  {
    node->setCode(str);
    codes[node->getSymbol()] = str;
  }
}

void rassokhina::Command::setQueue(std::vector< int > data, Command::priotity_queue_t& queue)
{
  for (int i = 0; i < data.size(); ++i)
  {
    if (data[i] != 0)
    {
      rassokhina::Node::node_t node = std::make_shared< rassokhina::Node >((unsigned char)i, data[i]);
      queue.push(node);
    }
  }
}

void rassokhina::Command::buildTree(Command::priotity_queue_t& queue)
{
  while (queue.size() > 1)
  {
    rassokhina::Node::node_t a = queue.top();
    queue.pop();
    rassokhina::Node::node_t b = queue.top();
    queue.pop();

    std::string name = a->getName() + b->getName();
    rassokhina::Node::node_t c = std::make_shared< rassokhina::Node >(name, a->getFrequency() + b->getFrequency());

    c->left_ = a;
    c->right_ = b;
    a->parent_ = c;
    b->parent_ = c;

    queue.push(c);
  }
}

std::string rassokhina::Command::textToCode(const std::string& text, const std::vector< std::string >& codes)
{
  std::string code;
  for (std::size_t i = 0; i < text.size(); ++i)
  {
    code += codes[static_cast< unsigned char >(text[i])];
  }
  return code;
}

std::string rassokhina::Command::codeToText(const std::string& text, const std::vector< std::string >& codes)
{
  std::vector< std::string >::const_iterator it = codes.begin();
  std::size_t minSize = 10;
  while (it != codes.end())
  {
    if (*it != "")
    {
      minSize = std::min(minSize, it->size());
    }
    ++it;
  }
  std::string::const_iterator it1 = text.begin();
  std::string code;
  std::string textChar;
  while (it1 != text.end())
  {
    code += *it1;
    ++it1;
    if (code.size() < minSize)
    {
      continue;
    }
    it = codes.begin();
    bool isCharAdd = false;
    int i = 0;
    while (it != codes.end())
    {
      if (code == *it)
      {
        textChar += static_cast< unsigned char >(i);
        isCharAdd = true;
        break;
      }
      ++i;
      ++it;
    }
    if (isCharAdd)
    {
      code.clear();
    }
  }
  return textChar;
}

std::string rassokhina::Command::doRead(std::istream& in, std::ostream& out)
{
  out << "text: ";
  std::string text;
  in.unsetf(std::ios_base::skipws);
  std::getline(in, text);
  return text;
}

std::string rassokhina::Command::doRead(const std::string& fileName)
{
  std::string text;
  std::ifstream file(fileName);
  if (!file)
  {
    throw std::invalid_argument("read: file not found");
  }
  file.unsetf(std::ios_base::skipws);
  std::copy(std::istream_iterator< char >(file), std::istream_iterator< char >(), std::back_inserter(text));
  return text;
}

void rassokhina::Command::doFlush(const std::string& text, std::ostream& out)
{
  std::copy(text.begin(), text.end(), std::ostream_iterator< char >(out));
  out << "\n";
}

void rassokhina::Command::doFlush(const std::string& text, const std::string& fileName)
{
  std::ofstream out(fileName);
  std::copy(text.begin(), text.end(), std::ostream_iterator< char >(out));
}
