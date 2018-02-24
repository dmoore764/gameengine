#include "xml.h"
#include "string.h"
#include "stdio.h"
#include "compiler_helper.h"

void XMLEatWhiteSpace(xmltokenizer *t)
{
	bool eating_whitespace = true;
	while (eating_whitespace)
	{
		if (t->at[0] == ' ' || t->at[0] == '\t' || t->at[0] == '\n' || t->at[0] == '\r')
			t->at++;
		else if (t->at[0] == '<' && t->at[1] == '?')
		{
			while (t->at[0] != '?' || t->at[1] != '>')
				t->at++;
			t->at += 2;
		}
		else
			eating_whitespace = false;
	}
};

xmlattr *XMLParseAttr(xmltokenizer *t, memory_arena *arena)
{
	XMLEatWhiteSpace(t);
	xmlattr *result = NULL;
	if (t->at[0] != '>' && strncmp(t->at, "/>", 2) != 0)
	{
		result = PUSH_ITEM(arena, xmlattr);
		
		char *nameStart = t->at;
		while (t->at[0] != '=')
		{
			t->at++;
		}
		result->name = PUSH_ARRAY(arena, char, (int)(t->at - nameStart) + 1);
		sprintf(result->name, "%.*s", (int)(t->at - nameStart), nameStart);
		
		t->at += 2;

		char *valStart = t->at;
		while (t->at[0] != '"' || (t->at[0] == '"' && t->at[-1] == '\\')) 
		{
			t->at++;
		}
		result->val = PUSH_ARRAY(arena, char, (int)(t->at - valStart) + 1);
		sprintf(result->val, "%.*s", (int)(t->at - valStart), valStart);

		t->at++;
	}
	return result;
}

xmlnode *XMLParseNode(xmltokenizer *t, memory_arena *arena)
{
	XMLEatWhiteSpace(t);
	
	if (strncmp(t->at, "</", 2) == 0)
		return NULL;

	assert(t->at[0] == '<');
	t->at++;

	xmlnode *result = PUSH_ITEM(arena, xmlnode);

	char *tagStart = t->at;
	//parse the node tag
	while (t->at[0] != ' ' && t->at[0] != '>' && t->at[0] != '/')
	{
		t->at++;
	}
	result->tag = PUSH_ARRAY(arena, char, (int)(t->at - tagStart) + 1);
	sprintf(result->tag, "%.*s", (int)(t->at - tagStart), tagStart);

	result->attrs = XMLParseAttr(t, arena);
	xmlattr *attr = result->attrs;
	while (attr)
	{
		attr->next = XMLParseAttr(t, arena);
		attr = attr->next;
	}

	XMLEatWhiteSpace(t);
	if (strncmp(t->at, "/>", 2) == 0)
	{
		result->children = NULL;
		result->value = NULL;
		t->at += 2;
	}
	else
	{
		assert(t->at[0] == '>');
		t->at++;
		char endTag[128];
		sprintf(endTag, "</%s>", result->tag);

		XMLEatWhiteSpace(t);
		if (t->at[0] != '<')
		{
			//text node
			char *tagEnd = strstr(t->at, endTag);
			result->value = PUSH_ARRAY(arena, char, (int)(tagEnd - t->at) + 1);
			sprintf(result->value, "%.*s", (int)(tagEnd - t->at), t->at);
			t->at = tagEnd + strlen(endTag);
		}
		else
		{
			result->children = XMLParseNode(t, arena);
			xmlnode *node = result->children;
			while (node)
			{
				node->next = XMLParseNode(t, arena);
				node = node->next;
			}
			assert(strncmp(t->at, endTag, strlen(endTag)) == 0);
			t->at += strlen(endTag);
		}
	}

	return result;
}

xmlnode *XMLGetFirstChildWithTag(xmlnode *parent, const char *tag)
{
	xmlnode *result = parent->children;
	while (result && strcmp(result->tag, tag) != 0)
	{
		result = result->next;
	}
	return result;
}

xmlnode *XMLGetNextChildWithTag(xmlnode *child, const char *tag)
{
	xmlnode *result = child->next;
	while (result && strcmp(result->tag, tag) != 0)
	{
		result = result->next;
	}
	return result;
}

int XMLNumChildrenWithTag(xmlnode *parent, const char *tag)
{
	int result = 0;
	xmlnode *current = parent->children;
	while (current)
	{
		if (strcmp(current->tag, tag) == 0)
			result++;
		current = current->next;
	}
	return result;
}

xmlattr *XMLGetAttrByName(xmlnode *node, const char *name)
{
	xmlattr *result = node->attrs;
	while (result && strcmp(result->name, name) != 0)
	{
		result = result->next;
	}
	return result;
}
