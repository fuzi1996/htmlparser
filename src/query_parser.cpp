#include "html_node.h"
#include "query_parser.h"

#include <iostream>
#include <wctype.h>
#include <algorithm>
#include <string>

PARSER_BEGIN_NAMESPACE

int query_parser::token(int n)
{
    return isalpha(n) || isdigit(n) || n == '-';
}

query_parser::query_parser() : token_parser()
{
//    _check_fns.push_back(&isalnum);
    _check_fns.push_back(&query_parser::token);
}

void query_parser::parse()
{
    query_rule_t *last_rule = new query_rule_t;

    std::vector<query_rule_t*> rl;
    for (size_t i = 0; i < _tokens.size(); ++i) {
        auto t = _tokens.at(i);
        std::cerr << t << std::endl;
        if (t == ",") {
            rl.push_back(last_rule);
            rules.push_back(rl);
            rl.clear();
            last_rule = new query_rule_t;
            continue;
        }

        if (t == ".") {
            ++i;
            last_rule->classes.push_back(_tokens.at(i));
            continue;
        }
        if (t == "#") {
            ++i;
            last_rule->id = _tokens.at(i);
            continue;
        }
        if (t == ">") {
            rl.push_back(last_rule);
            last_rule = new query_rule_t;
            last_rule->is_child = true;
            continue;
        }

        if (last_rule->is_valid())
            rl.push_back(last_rule);
        last_rule = new query_rule_t;
        last_rule->is_child = false;
        last_rule->tag_name = t;

    }
    rl.push_back(last_rule);

    rules.push_back(rl);
}

html_tag_vector query_parser::search()
{
    html_tag_vector tags;
    for (auto l : rules) {
        size_t i = 0;
        search(&tags, tag, i, l);
    }
    return tags;
}

void query_parser::search(html_tag_vector *tags, html_tag *tag, size_t rule_id, std::vector<query_rule_t *> rules)
{
    if (rule_id >= rules.size()) {
        return;
    }

    auto rule = rules.at(rule_id);
    if (rule->check(tag)) {

        if (rule_id == rules.size() - 1)
            tags->push_back(tag);
        rule_id++;
    }

    for (auto child : tag->childs()) {
        auto t = dynamic_cast<html_tag*>(child);
        if (t)
            search(tags, t, rule_id, rules);
    }
}

bool query_parser::query_rule_t::is_valid() const {
    return id.size() || tag_name.size() || classes.size();
}

bool query_parser::query_rule_t::check(html_tag *tag) const
{
    if (!tag)
        return false;

    if (id.size() && tag->id() != id)
        return false;

    if (tag_name.size() && tag->name != tag_name)
        return false;

    if (classes.size())
        for (auto cls : classes) {
            if (std::none_of(tag->_classes.begin(), tag->_classes.end(), [=](std::string c){
                return c == cls;
            })){
                return false;
            }
        }

    return true;
}

query_parser::query_rule_t::query_rule_t() : is_child(false)
{

}

PARSER_END_NAMESPACE
