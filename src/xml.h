#pragma once
#include "memory_arena.h"

struct xmlattr
{
	char *name;
	char *val;
	xmlattr *next;
};

struct xmlnode
{
	char *tag;
	xmlattr *attrs;
	char *value;

	xmlnode *children;
	xmlnode *next;
};

struct xmltokenizer
{
	char *at;
};

xmlattr *XMLParseAttr(xmltokenizer *t, memory_arena *arena);
xmlnode *XMLParseNode(xmltokenizer *t, memory_arena *arena);
xmlnode *XMLGetFirstChildWithTag(xmlnode *parent, const char *tag);
xmlnode *XMLGetNextChildWithTag(xmlnode *child, const char *tag);
int XMLNumChildrenWithTag(xmlnode *parent, const char *tag);
xmlattr *XMLGetAttrByName(xmlnode *node, const char *name);
