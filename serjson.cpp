//
// Created by Matteo Cardinaletti on 12/05/23.
//

#include "serjson.h"

#include <utility>
#include <iostream>

void trim_left(std::string &str) {
    while (str.starts_with(" ") || str.starts_with("\t")) {
        str.erase(0, 1);
    }
}

std::vector<std::string> tokenize(std::string &source) {
    std::vector<std::string> v;
    while (!source.empty()) {
        bool is_key = false;
        trim_left(source);

        size_t next_pos = source.find(' ');
        if (next_pos == std::string::npos) {
            if (!source.empty()) {
                v.push_back(source);
                source.clear();
            }
            break;
        }

        std::string str = source.substr(0, next_pos);

        if (str.ends_with(",")) str.pop_back();

        if (str.ends_with(":")) {
            is_key = true;
            str.pop_back();
        }

        if (!str.empty()) v.push_back(str);
        if (is_key) v.emplace_back(":");
        source.erase(0, next_pos);
    }

    return v;
}

std::string pop(std::vector<std::string> &v) {
    if (v.empty()) return "";
    std::string s = v.back();
    v.pop_back();
    return s;
}

void unquote(std::string &str) {
    if (!str.starts_with('"') || !str.ends_with('"')) return;

    str.erase(0, 1);
    str.pop_back();
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
void parse(std::vector<std::string> &tokens, std::vector<json::Node> &target) {
    if (tokens.back() != "{" && tokens.back() != "[") return;

    // FIXME: this will not parse a file containing only an array like [ 10, 20, 30 ] or [ {}, {}, {} ] for example

    bool is_array = false;
    if (tokens.back() == "[") is_array = true;

    pop(tokens);

    while (!tokens.empty() && tokens.back() != "}" && tokens.back() != "]") {
        json::Node node;
        std::string str;

        // parsing key
        if (!is_array) {
            if (tokens.back().starts_with('"')) {
                str = pop(tokens);
                unquote(str);
                node.key = str;
            }

            if (tokens.back() != ":") return;
            pop(tokens);
        } else node.key = "";

        // cleanest way to implement a sort of switch inside a while loop
        if (tokens.back().starts_with('"')) {
            node.type = json::string;
            str = pop(tokens);
            unquote(str);
            node.value = str;
            target.push_back(node);
            continue;
        }

        if (std::isdigit(tokens.back()[0])) {
            node.type = json::number;
            node.value = std::stof(pop(tokens));
            target.push_back(node);
            continue;
        }

        if (tokens.back() == "true" || tokens.back() == "false") {
            node.type = json::boolean;
            node.value = (pop(tokens) == "true");
            target.push_back(node);
            continue;
        }

        if (tokens.back() == "null") {
            node.type = json::null;
            node.value = pop(tokens);
            target.push_back(node);
            continue;
        }

        if (tokens.back() == "{") {
            node.type = json::object;
            std::vector<json::Node> attributes;
            parse(tokens, attributes);
            node.value = attributes;
            target.push_back(node);
            continue;
        }

        if (tokens.back() == "[") {
            node.type = json::array;
            std::vector<json::Node> items;
            parse(tokens, items);
            node.value = items;
            target.push_back(node);
            continue;
        }
    }

    if (tokens.back() == "}" || tokens.back() == "]")
        pop(tokens);
}
#pragma clang diagnostic pop


json::Object json::read(const std::string &path) {
    json::Object json {.file_path = path, .nodes = {}};

    std::string text;

    std::ifstream file;
    if (!file.is_open()) file.open(path);
    if (file.is_open()) {
        std::string line;
        while (file.good()) {
            std::getline(file, line);
            text += line + " ";
        }
    }
    file.close();

    if (text.empty()) return json;

    // FIXME: tokenize func will not split non-formatted jsons
    std::vector<std::string> tokens = tokenize(text);
    if (tokens.empty()) return json;

    //reverse the tokens to treat them as a stack but with the advantages of a vector
    std::reverse(tokens.begin(), tokens.end());

    parse(tokens, json.nodes);

    return json;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
std::string json_node_to_string(json::Node &node) {
    std::string str;
    if (!node.key.empty())
        str += "\"" + node.key + "\": ";
    switch (node.type) {
        case json::string:
            str += "\"" + node.get_value<std::string>() + "\"";
            break;
        case json::number:
            str += std::to_string(node.get_value<float>());
            break;
        case json::object:
        case json::array:
            {
                auto nodes = node.get_value<std::vector<json::Node>>();
                if (nodes.empty()) {
                    str += "null";
                    break;
                }

                if (node.type == json::object) str += "{ ";
                else str += "[ ";

                for (int i = 0; i < nodes.size(); ++i) {
                    str += json_node_to_string(nodes[i]);
                    if (i < nodes.size() - 1) str += ", ";
                }

                if (node.type == json::object) str += " }";
                else str += " ]";
            }
            break;
        case json::null:
            str += "null";
            break;
        case json::boolean:
            str += (node.get_value<bool>() ? "true" : "false");
            break;
        default:
            break;
    }

    return str;
}
#pragma clang diagnostic pop

json::Node json::generate_parent(std::string key, std::vector<json::Node> children) {
    json::Node parent;
    parent.type = json::object;
    parent.key = std::move(key);
    parent.value = children;
    return parent;
}

void json::add_node(json::Serializable &obj, json::Object &json) {
    json.nodes.push_back(obj.serialize());
}

void json::add_node(json::Node &obj, json::Object &json) {
    json.nodes.push_back(obj);
}

void json::write(json::Object &json) {
    std::string str = "{ ";
    for (int i = 0; i < json.nodes.size(); ++i) {
        str += json_node_to_string(json.nodes[i]);
        if (i < json.nodes.size() - 1) str += ", ";
    }
    str += " }";

    std::ofstream output_file(json.file_path);
    output_file << str;
    output_file.close();
}