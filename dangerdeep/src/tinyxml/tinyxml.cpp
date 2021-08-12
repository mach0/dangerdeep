/*
www.sourceforge.net/projects/tinyxml
Original code by Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#include <cctype>

#ifdef TIXML_USE_STL
#include <iostream>
#include <sstream>
#endif

#include "tinyxml.h"

auto TiXmlFOpen(const char* filename, const char* mode) -> FILE*;

bool TiXmlBase::condenseWhiteSpace = true;

// Microsoft compiler security
auto TiXmlFOpen(const char* filename, const char* mode) -> FILE*
{
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    FILE* fp    = 0;
    errno_t err = fopen_s(&fp, filename, mode);
    if (!err && fp)
        return fp;
    return 0;
#else
    return fopen(filename, mode);
#endif
}

void TiXmlBase::EncodeString(const TIXML_STRING& str, TIXML_STRING* outString)
{
    int i = 0;

    while (i < static_cast<int>(str.length()))
    {
        auto c = static_cast<unsigned char>(str[i]);

        if (c == '&' && i < (static_cast<int>(str.length()) - 2) && str[i + 1] == '#'
            && str[i + 2] == 'x')
        {
            // Hexadecimal character reference.
            // Pass through unchanged.
            // &#xA9;	-- copyright symbol, for example.
            //
            // The -1 is a bug fix from Rob Laveaux. It keeps
            // an overflow from happening if there is no ';'.
            // There are actually 2 ways to exit this loop -
            // while fails (error case) and break (semicolon found).
            // However, there is no mechanism (currently) for
            // this function to return an error.
            while (i < static_cast<int>(str.length()) - 1)
            {
                outString->append(str.c_str() + i, 1);
                ++i;
                if (str[i] == ';')
                {
                    break;
                }
            }
        }
        else if (c == '&')
        {
            outString->append(entity[0].str, entity[0].strLength);
            ++i;
        }
        else if (c == '<')
        {
            outString->append(entity[1].str, entity[1].strLength);
            ++i;
        }
        else if (c == '>')
        {
            outString->append(entity[2].str, entity[2].strLength);
            ++i;
        }
        else if (c == '\"')
        {
            outString->append(entity[3].str, entity[3].strLength);
            ++i;
        }
        else if (c == '\'')
        {
            outString->append(entity[4].str, entity[4].strLength);
            ++i;
        }
        else if (c < 32)
        {
            // Easy pass at non-alpha/numeric/symbol
            // Below 32 is symbolic.
            char buf[32];

#if defined(TIXML_SNPRINTF)
            TIXML_SNPRINTF(buf, sizeof(buf), "&#x%02X;", static_cast<unsigned>(c & 0xff));
#else
            sprintf(buf, "&#x%02X;", (unsigned) (c & 0xff));
#endif

            //*ME:	warning C4267: convert 'size_t' to 'int'
            //*ME:	Int-Cast to make compiler happy ...
            outString->append(buf, static_cast<int>(strlen(buf)));
            ++i;
        }
        else
        {
            // char realc = (char) c;
            // outString->append( &realc, 1 );
            *outString += static_cast<char>(c); // somewhat more efficient function call.
            ++i;
        }
    }
}

TiXmlNode::TiXmlNode(NodeType _type) : TiXmlBase()
{
    parent     = nullptr;
    type       = _type;
    firstChild = nullptr;
    lastChild  = nullptr;
    prev       = nullptr;
    next       = nullptr;
}

TiXmlNode::~TiXmlNode()
{
    TiXmlNode* node = firstChild;
    TiXmlNode* temp = nullptr;

    while (node)
    {
        temp = node;
        node = node->next;
        delete temp;
    }
}

void TiXmlNode::CopyTo(TiXmlNode* target) const
{
    target->SetValue(value.c_str());
    target->userData = userData;
    target->location = location;
}

void TiXmlNode::Clear()
{
    TiXmlNode* node = firstChild;
    TiXmlNode* temp = nullptr;

    while (node)
    {
        temp = node;
        node = node->next;
        delete temp;
    }

    firstChild = nullptr;
    lastChild  = nullptr;
}

auto TiXmlNode::LinkEndChild(TiXmlNode* node) -> TiXmlNode*
{
    assert(node->parent == nullptr || node->parent == this);
    assert(
        node->GetDocument() == nullptr
        || node->GetDocument() == this->GetDocument());

    if (node->Type() == TiXmlNode::TINYXML_DOCUMENT)
    {
        delete node;
        if (GetDocument())
        {
            GetDocument()->SetError(
                TIXML_ERROR_DOCUMENT_TOP_ONLY,
                nullptr,
                nullptr,
                TIXML_ENCODING_UNKNOWN);
        }
        return nullptr;
    }

    node->parent = this;

    node->prev = lastChild;
    node->next = nullptr;

    if (lastChild)
    {
        lastChild->next = node;
    }
    else
    {
        firstChild = node; // it was an empty list.
    }

    lastChild = node;
    return node;
}

auto TiXmlNode::InsertEndChild(const TiXmlNode& addThis) -> TiXmlNode*
{
    if (addThis.Type() == TiXmlNode::TINYXML_DOCUMENT)
    {
        if (GetDocument())
        {
            GetDocument()->SetError(
                TIXML_ERROR_DOCUMENT_TOP_ONLY,
                nullptr,
                nullptr,
                TIXML_ENCODING_UNKNOWN);
        }
        return nullptr;
    }
    TiXmlNode* node = addThis.Clone();
    if (!node)
    {
        return nullptr;
    }

    return LinkEndChild(node);
}

auto TiXmlNode::InsertBeforeChild(
    TiXmlNode* beforeThis,
    const TiXmlNode& addThis) -> TiXmlNode*
{
    if (!beforeThis || beforeThis->parent != this)
    {
        return nullptr;
    }
    if (addThis.Type() == TiXmlNode::TINYXML_DOCUMENT)
    {
        if (GetDocument())
        {
            GetDocument()->SetError(
                TIXML_ERROR_DOCUMENT_TOP_ONLY,
                nullptr,
                nullptr,
                TIXML_ENCODING_UNKNOWN);
        }
        return nullptr;
    }

    TiXmlNode* node = addThis.Clone();
    if (!node)
    {
        return nullptr;
    }
    node->parent = this;

    node->next = beforeThis;
    node->prev = beforeThis->prev;
    if (beforeThis->prev)
    {
        beforeThis->prev->next = node;
    }
    else
    {
        assert(firstChild == beforeThis);
        firstChild = node;
    }
    beforeThis->prev = node;
    return node;
}

auto TiXmlNode::InsertAfterChild(TiXmlNode* afterThis, const TiXmlNode& addThis)
    -> TiXmlNode*
{
    if (!afterThis || afterThis->parent != this)
    {
        return nullptr;
    }
    if (addThis.Type() == TiXmlNode::TINYXML_DOCUMENT)
    {
        if (GetDocument())
        {
            GetDocument()->SetError(
                TIXML_ERROR_DOCUMENT_TOP_ONLY,
                nullptr,
                nullptr,
                TIXML_ENCODING_UNKNOWN);
        }
        return nullptr;
    }

    TiXmlNode* node = addThis.Clone();
    if (!node)
    {
        return nullptr;
    }
    node->parent = this;

    node->prev = afterThis;
    node->next = afterThis->next;
    if (afterThis->next)
    {
        afterThis->next->prev = node;
    }
    else
    {
        assert(lastChild == afterThis);
        lastChild = node;
    }
    afterThis->next = node;
    return node;
}

auto TiXmlNode::ReplaceChild(TiXmlNode* replaceThis, const TiXmlNode& withThis)
    -> TiXmlNode*
{
    if (!replaceThis)
    {
        return nullptr;
    }

    if (replaceThis->parent != this)
    {
        return nullptr;
    }

    if (withThis.ToDocument())
    {
        // A document can never be a child.	Thanks to Noam.
        TiXmlDocument* document = GetDocument();
        if (document)
        {
            document->SetError(
                TIXML_ERROR_DOCUMENT_TOP_ONLY,
                nullptr,
                nullptr,
                TIXML_ENCODING_UNKNOWN);
        }
        return nullptr;
    }

    TiXmlNode* node = withThis.Clone();
    if (!node)
    {
        return nullptr;
    }

    node->next = replaceThis->next;
    node->prev = replaceThis->prev;

    if (replaceThis->next)
    {
        replaceThis->next->prev = node;
    }
    else
    {
        lastChild = node;
    }

    if (replaceThis->prev)
    {
        replaceThis->prev->next = node;
    }
    else
    {
        firstChild = node;
    }

    delete replaceThis;
    node->parent = this;
    return node;
}

auto TiXmlNode::RemoveChild(TiXmlNode* removeThis) -> bool
{
    if (!removeThis)
    {
        return false;
    }

    if (removeThis->parent != this)
    {
        assert(0);
        return false;
    }

    if (removeThis->next)
    {
        removeThis->next->prev = removeThis->prev;
    }
    else
    {
        lastChild = removeThis->prev;
    }

    if (removeThis->prev)
    {
        removeThis->prev->next = removeThis->next;
    }
    else
    {
        firstChild = removeThis->next;
    }

    delete removeThis;
    return true;
}

auto TiXmlNode::FirstChild(const char* _value) const -> const TiXmlNode*
{
    const TiXmlNode* node;
    for (node = firstChild; node; node = node->next)
    {
        if (strcmp(node->Value(), _value) == 0)
        {
            return node;
        }
    }
    return nullptr;
}

auto TiXmlNode::LastChild(const char* _value) const -> const TiXmlNode*
{
    const TiXmlNode* node;
    for (node = lastChild; node; node = node->prev)
    {
        if (strcmp(node->Value(), _value) == 0)
        {
            return node;
        }
    }
    return nullptr;
}

auto TiXmlNode::IterateChildren(const TiXmlNode* previous) const
    -> const TiXmlNode*
{
    if (!previous)
    {
        return FirstChild();
    }
    else
    {
        assert(previous->parent == this);
        return previous->NextSibling();
    }
}

auto TiXmlNode::IterateChildren(const char* val, const TiXmlNode* previous)
    const -> const TiXmlNode*
{
    if (!previous)
    {
        return FirstChild(val);
    }
    else
    {
        assert(previous->parent == this);
        return previous->NextSibling(val);
    }
}

auto TiXmlNode::NextSibling(const char* _value) const -> const TiXmlNode*
{
    const TiXmlNode* node;
    for (node = next; node; node = node->next)
    {
        if (strcmp(node->Value(), _value) == 0)
        {
            return node;
        }
    }
    return nullptr;
}

auto TiXmlNode::PreviousSibling(const char* _value) const -> const TiXmlNode*
{
    const TiXmlNode* node;
    for (node = prev; node; node = node->prev)
    {
        if (strcmp(node->Value(), _value) == 0)
        {
            return node;
        }
    }
    return nullptr;
}

void TiXmlElement::RemoveAttribute(const char* name)
{
#ifdef TIXML_USE_STL
    TIXML_STRING str(name);
    TiXmlAttribute* node = attributeSet.Find(str);
#else
    TiXmlAttribute* node = attributeSet.Find(name);
#endif
    if (node)
    {
        attributeSet.Remove(node);
        delete node;
    }
}

auto TiXmlNode::FirstChildElement() const -> const TiXmlElement*
{
    const TiXmlNode* node;

    for (node = FirstChild(); node; node = node->NextSibling())
    {
        if (node->ToElement())
        {
            return node->ToElement();
        }
    }
    return nullptr;
}

auto TiXmlNode::FirstChildElement(const char* _value) const
    -> const TiXmlElement*
{
    const TiXmlNode* node;

    for (node = FirstChild(_value); node; node = node->NextSibling(_value))
    {
        if (node->ToElement())
        {
            return node->ToElement();
        }
    }
    return nullptr;
}

auto TiXmlNode::NextSiblingElement() const -> const TiXmlElement*
{
    const TiXmlNode* node;

    for (node = NextSibling(); node; node = node->NextSibling())
    {
        if (node->ToElement())
        {
            return node->ToElement();
        }
    }
    return nullptr;
}

auto TiXmlNode::NextSiblingElement(const char* _value) const
    -> const TiXmlElement*
{
    const TiXmlNode* node;

    for (node = NextSibling(_value); node; node = node->NextSibling(_value))
    {
        if (node->ToElement())
        {
            return node->ToElement();
        }
    }
    return nullptr;
}

auto TiXmlNode::GetDocument() const -> const TiXmlDocument*
{
    const TiXmlNode* node;

    for (node = this; node; node = node->parent)
    {
        if (node->ToDocument())
        {
            return node->ToDocument();
        }
    }
    return nullptr;
}

TiXmlElement::TiXmlElement(const char* _value) :
    TiXmlNode(TiXmlNode::TINYXML_ELEMENT)
{
    firstChild = lastChild = nullptr;
    value                  = _value;
}

#ifdef TIXML_USE_STL
TiXmlElement::TiXmlElement(const std::string& _value) :
    TiXmlNode(TiXmlNode::TINYXML_ELEMENT)
{
    firstChild = lastChild = nullptr;
    value                  = _value;
}
#endif

TiXmlElement::TiXmlElement(const TiXmlElement& copy) :
    TiXmlNode(TiXmlNode::TINYXML_ELEMENT)
{
    firstChild = lastChild = nullptr;
    copy.CopyTo(this);
}

auto TiXmlElement::operator=(const TiXmlElement& base) -> TiXmlElement&
{
    ClearThis();
    base.CopyTo(this);
    return *this;
}

TiXmlElement::~TiXmlElement()
{
    ClearThis();
}

void TiXmlElement::ClearThis()
{
    Clear();
    while (attributeSet.First())
    {
        TiXmlAttribute* node = attributeSet.First();
        attributeSet.Remove(node);
        delete node;
    }
}

auto TiXmlElement::Attribute(const char* name) const -> const char*
{
    const TiXmlAttribute* node = attributeSet.Find(name);
    if (node)
    {
        return node->Value();
    }
    return nullptr;
}

#ifdef TIXML_USE_STL
auto TiXmlElement::Attribute(const std::string& name) const
    -> const std::string*
{
    const TiXmlAttribute* attrib = attributeSet.Find(name);
    if (attrib)
    {
        return &attrib->ValueStr();
    }
    return nullptr;
}
#endif

auto TiXmlElement::Attribute(const char* name, int* i) const -> const char*
{
    const TiXmlAttribute* attrib = attributeSet.Find(name);
    const char* result           = nullptr;

    if (attrib)
    {
        result = attrib->Value();
        if (i)
        {
            attrib->QueryIntValue(i);
        }
    }
    return result;
}

#ifdef TIXML_USE_STL
auto TiXmlElement::Attribute(const std::string& name, int* i) const
    -> const std::string*
{
    const TiXmlAttribute* attrib = attributeSet.Find(name);
    const std::string* result    = nullptr;

    if (attrib)
    {
        result = &attrib->ValueStr();
        if (i)
        {
            attrib->QueryIntValue(i);
        }
    }
    return result;
}
#endif

auto TiXmlElement::Attribute(const char* name, double* d) const -> const char*
{
    const TiXmlAttribute* attrib = attributeSet.Find(name);
    const char* result           = nullptr;

    if (attrib)
    {
        result = attrib->Value();
        if (d)
        {
            attrib->QueryDoubleValue(d);
        }
    }
    return result;
}

#ifdef TIXML_USE_STL
auto TiXmlElement::Attribute(const std::string& name, double* d) const
    -> const std::string*
{
    const TiXmlAttribute* attrib = attributeSet.Find(name);
    const std::string* result    = nullptr;

    if (attrib)
    {
        result = &attrib->ValueStr();
        if (d)
        {
            attrib->QueryDoubleValue(d);
        }
    }
    return result;
}
#endif

auto TiXmlElement::QueryIntAttribute(const char* name, int* ival) const -> int
{
    const TiXmlAttribute* attrib = attributeSet.Find(name);
    if (!attrib)
    {
        return TIXML_NO_ATTRIBUTE;
    }
    return attrib->QueryIntValue(ival);
}

auto TiXmlElement::QueryUnsignedAttribute(const char* name, unsigned* value)
    const -> int
{
    const TiXmlAttribute* node = attributeSet.Find(name);
    if (!node)
    {
        return TIXML_NO_ATTRIBUTE;
    }

    int ival   = 0;
    int result = node->QueryIntValue(&ival);
    *value     = static_cast<unsigned>(ival);
    return result;
}

auto TiXmlElement::QueryBoolAttribute(const char* name, bool* bval) const -> int
{
    const TiXmlAttribute* node = attributeSet.Find(name);
    if (!node)
    {
        return TIXML_NO_ATTRIBUTE;
    }

    int result = TIXML_WRONG_TYPE;
    if (StringEqual(node->Value(), "true", true, TIXML_ENCODING_UNKNOWN)
        || StringEqual(node->Value(), "yes", true, TIXML_ENCODING_UNKNOWN)
        || StringEqual(node->Value(), "1", true, TIXML_ENCODING_UNKNOWN))
    {
        *bval  = true;
        result = TIXML_SUCCESS;
    }
    else if (
        StringEqual(node->Value(), "false", true, TIXML_ENCODING_UNKNOWN)
        || StringEqual(node->Value(), "no", true, TIXML_ENCODING_UNKNOWN)
        || StringEqual(node->Value(), "0", true, TIXML_ENCODING_UNKNOWN))
    {
        *bval  = false;
        result = TIXML_SUCCESS;
    }
    return result;
}

#ifdef TIXML_USE_STL
auto TiXmlElement::QueryIntAttribute(const std::string& name, int* ival) const
    -> int
{
    const TiXmlAttribute* attrib = attributeSet.Find(name);
    if (!attrib)
    {
        return TIXML_NO_ATTRIBUTE;
    }
    return attrib->QueryIntValue(ival);
}
#endif

auto TiXmlElement::QueryDoubleAttribute(const char* name, double* dval) const
    -> int
{
    const TiXmlAttribute* attrib = attributeSet.Find(name);
    if (!attrib)
    {
        return TIXML_NO_ATTRIBUTE;
    }
    return attrib->QueryDoubleValue(dval);
}

#ifdef TIXML_USE_STL
auto TiXmlElement::QueryDoubleAttribute(const std::string& name, double* dval)
    const -> int
{
    const TiXmlAttribute* attrib = attributeSet.Find(name);
    if (!attrib)
    {
        return TIXML_NO_ATTRIBUTE;
    }
    return attrib->QueryDoubleValue(dval);
}
#endif

void TiXmlElement::SetAttribute(const char* name, int val)
{
    TiXmlAttribute* attrib = attributeSet.FindOrCreate(name);
    if (attrib)
    {
        attrib->SetIntValue(val);
    }
}

#ifdef TIXML_USE_STL
void TiXmlElement::SetAttribute(const std::string& name, int val)
{
    TiXmlAttribute* attrib = attributeSet.FindOrCreate(name);
    if (attrib)
    {
        attrib->SetIntValue(val);
    }
}
#endif

void TiXmlElement::SetDoubleAttribute(const char* name, double val)
{
    TiXmlAttribute* attrib = attributeSet.FindOrCreate(name);
    if (attrib)
    {
        attrib->SetDoubleValue(val);
    }
}

#ifdef TIXML_USE_STL
void TiXmlElement::SetDoubleAttribute(const std::string& name, double val)
{
    TiXmlAttribute* attrib = attributeSet.FindOrCreate(name);
    if (attrib)
    {
        attrib->SetDoubleValue(val);
    }
}
#endif

void TiXmlElement::SetAttribute(const char* cname, const char* cvalue)
{
    TiXmlAttribute* attrib = attributeSet.FindOrCreate(cname);
    if (attrib)
    {
        attrib->SetValue(cvalue);
    }
}

#ifdef TIXML_USE_STL
void TiXmlElement::SetAttribute(
    const std::string& _name,
    const std::string& _value)
{
    TiXmlAttribute* attrib = attributeSet.FindOrCreate(_name);
    if (attrib)
    {
        attrib->SetValue(_value);
    }
}
#endif

void TiXmlElement::Print(FILE* cfile, int depth) const
{
    int i;
    assert(cfile);
    for (i = 0; i < depth; i++)
    {
        fprintf(cfile, "    ");
    }

    fprintf(cfile, "<%s", value.c_str());

    const TiXmlAttribute* attrib;
    for (attrib = attributeSet.First(); attrib; attrib = attrib->Next())
    {
        fprintf(cfile, " ");
        attrib->Print(cfile, depth);
    }

    // There are 3 different formatting approaches:
    // 1) An element without children is printed as a <foo /> node
    // 2) An element with only a text child is printed as <foo> text </foo>
    // 3) An element with children is printed on multiple lines.
    TiXmlNode* node;
    if (!firstChild)
    {
        fprintf(cfile, " />");
    }
    else if (firstChild == lastChild && firstChild->ToText())
    {
        fprintf(cfile, ">");
        firstChild->Print(cfile, depth + 1);
        fprintf(cfile, "</%s>", value.c_str());
    }
    else
    {
        fprintf(cfile, ">");

        for (node = firstChild; node; node = node->NextSibling())
        {
            if (!node->ToText())
            {
                fprintf(cfile, "\n");
            }
            node->Print(cfile, depth + 1);
        }
        fprintf(cfile, "\n");
        for (i = 0; i < depth; ++i)
        {
            fprintf(cfile, "    ");
        }
        fprintf(cfile, "</%s>", value.c_str());
    }
}

void TiXmlElement::CopyTo(TiXmlElement* target) const
{
    // superclass:
    TiXmlNode::CopyTo(target);

    // Element class:
    // Clone the attributes, then clone the children.
    const TiXmlAttribute* attribute = nullptr;
    for (attribute = attributeSet.First(); attribute;
         attribute = attribute->Next())
    {
        target->SetAttribute(attribute->Name(), attribute->Value());
    }

    TiXmlNode* node = nullptr;
    for (node = firstChild; node; node = node->NextSibling())
    {
        target->LinkEndChild(node->Clone());
    }
}

auto TiXmlElement::Accept(TiXmlVisitor* visitor) const -> bool
{
    if (visitor->VisitEnter(*this, attributeSet.First()))
    {
        for (const TiXmlNode* node = FirstChild(); node;
             node                  = node->NextSibling())
        {
            if (!node->Accept(visitor))
            {
                break;
            }
        }
    }
    return visitor->VisitExit(*this);
}

auto TiXmlElement::Clone() const -> TiXmlNode*
{
    auto* clone = new TiXmlElement(Value());
    if (!clone)
    {
        return nullptr;
    }

    CopyTo(clone);
    return clone;
}

auto TiXmlElement::GetText() const -> const char*
{
    const TiXmlNode* child = this->FirstChild();
    if (child)
    {
        const TiXmlText* childText = child->ToText();
        if (childText)
        {
            return childText->Value();
        }
    }
    return nullptr;
}

TiXmlDocument::TiXmlDocument() : TiXmlNode(TiXmlNode::TINYXML_DOCUMENT)
{
    tabsize         = 4;
    useMicrosoftBOM = false;
    ClearError();
}

TiXmlDocument::TiXmlDocument(const char* documentName) :
    TiXmlNode(TiXmlNode::TINYXML_DOCUMENT)
{
    tabsize         = 4;
    useMicrosoftBOM = false;
    value           = documentName;
    ClearError();
}

#ifdef TIXML_USE_STL
TiXmlDocument::TiXmlDocument(const std::string& documentName) :
    TiXmlNode(TiXmlNode::TINYXML_DOCUMENT)
{
    tabsize         = 4;
    useMicrosoftBOM = false;
    value           = documentName;
    ClearError();
}
#endif

TiXmlDocument::TiXmlDocument(const TiXmlDocument& copy) :
    TiXmlNode(TiXmlNode::TINYXML_DOCUMENT)
{
    copy.CopyTo(this);
}

auto TiXmlDocument::operator=(const TiXmlDocument& copy) -> TiXmlDocument&
{
    Clear();
    copy.CopyTo(this);
    return *this;
}

auto TiXmlDocument::LoadFile(TiXmlEncoding encoding) -> bool
{
    return LoadFile(Value(), encoding);
}

auto TiXmlDocument::SaveFile() const -> bool
{
    return SaveFile(Value());
}

auto TiXmlDocument::LoadFile(const char* _filename, TiXmlEncoding encoding)
    -> bool
{
    TIXML_STRING filename(_filename);
    value = filename;

    // reading in binary mode so that tinyxml can normalize the EOL
    FILE* file = TiXmlFOpen(value.c_str(), "rb");

    if (file)
    {
        bool result = LoadFile(file, encoding);
        fclose(file);
        return result;
    }
    else
    {
        SetError(
            TIXML_ERROR_OPENING_FILE, nullptr, nullptr, TIXML_ENCODING_UNKNOWN);
        return false;
    }
}

auto TiXmlDocument::LoadFile(FILE* file, TiXmlEncoding encoding) -> bool
{
    if (!file)
    {
        SetError(
            TIXML_ERROR_OPENING_FILE, nullptr, nullptr, TIXML_ENCODING_UNKNOWN);
        return false;
    }

    // Delete the existing data:
    Clear();
    location.Clear();

    // Get the file size, so we can pre-allocate the string. HUGE speed impact.
    long length = 0;
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Strange case, but good to handle up front.
    if (length <= 0)
    {
        SetError(
            TIXML_ERROR_DOCUMENT_EMPTY,
            nullptr,
            nullptr,
            TIXML_ENCODING_UNKNOWN);
        return false;
    }

    // Subtle bug here. TinyXml did use fgets. But from the XML spec:
    // 2.11 End-of-Line Handling
    // <snip>
    // <quote>
    // ...the XML processor MUST behave as if it normalized all line breaks in
    // external parsed entities (including the document entity) on input, before
    // parsing, by translating both the two-character sequence #xD #xA and any
    // #xD that is not followed by #xA to a single #xA character.
    // </quote>
    //
    // It is not clear fgets does that, and certainly isn't clear it works cross
    // platform. Generally, you expect fgets to translate from the convention of
    // the OS to the c/unix convention, and not work generally.

    /*
    while( fgets( buf, sizeof(buf), file ) )
    {
        data += buf;
    }
    */

    char* buf = new char[length + 1];
    buf[0]    = 0;

    if (fread(buf, length, 1, file) != 1)
    {
        delete[] buf;
        SetError(
            TIXML_ERROR_OPENING_FILE, nullptr, nullptr, TIXML_ENCODING_UNKNOWN);
        return false;
    }

    // Process the buffer in place to normalize new lines. (See comment above.)
    // Copies from the 'p' to 'q' pointer, where p can advance faster if
    // a newline-carriage return is hit.
    //
    // Wikipedia:
    // Systems based on ASCII or a compatible character set use either LF  (Line
    // feed, '\n', 0x0A, 10 in decimal) or CR (Carriage return, '\r', 0x0D, 13
    // in decimal) individually, or CR followed by LF (CR+LF, 0x0D 0x0A)...
    //		* LF:    Multics, Unix and Unix-like systems (GNU/Linux, AIX, Xenix,
    // Mac OS X, FreeBSD, etc.), BeOS, Amiga, RISC OS, and others
    //		* CR+LF: DEC RT-11 and most other early non-Unix, non-IBM OSes,
    // CP/M, MP/M, DOS, OS/2, Microsoft Windows, Symbian OS
    //		* CR:    Commodore 8-bit machines, Apple II family, Mac OS up to
    // version 9 and OS-9

    const char* p = buf; // the read head
    char* q       = buf; // the write head
    const char CR = 0x0d;
    const char LF = 0x0a;

    buf[length] = 0;
    while (*p)
    {
        assert(p < (buf + length));
        assert(q <= (buf + length));
        assert(q <= p);

        if (*p == CR)
        {
            *q++ = LF;
            p++;
            if (*p == LF)
            { // check for CR+LF (and skip LF)
                p++;
            }
        }
        else
        {
            *q++ = *p++;
        }
    }
    assert(q <= (buf + length));
    *q = 0;

    Parse(buf, nullptr, encoding);

    delete[] buf;
    return !Error();
}

auto TiXmlDocument::SaveFile(const char* filename) const -> bool
{
    // The old c stuff lives on...
    FILE* fp = TiXmlFOpen(filename, "w");
    if (fp)
    {
        bool result = SaveFile(fp);
        fclose(fp);
        return result;
    }
    return false;
}

auto TiXmlDocument::SaveFile(FILE* fp) const -> bool
{
    if (useMicrosoftBOM)
    {
        const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
        const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
        const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;

        fputc(TIXML_UTF_LEAD_0, fp);
        fputc(TIXML_UTF_LEAD_1, fp);
        fputc(TIXML_UTF_LEAD_2, fp);
    }
    Print(fp, 0);
    return (ferror(fp) == 0);
}

void TiXmlDocument::CopyTo(TiXmlDocument* target) const
{
    TiXmlNode::CopyTo(target);

    target->error           = error;
    target->errorId         = errorId;
    target->errorDesc       = errorDesc;
    target->tabsize         = tabsize;
    target->errorLocation   = errorLocation;
    target->useMicrosoftBOM = useMicrosoftBOM;

    TiXmlNode* node = nullptr;
    for (node = firstChild; node; node = node->NextSibling())
    {
        target->LinkEndChild(node->Clone());
    }
}

auto TiXmlDocument::Clone() const -> TiXmlNode*
{
    auto* clone = new TiXmlDocument();
    if (!clone)
    {
        return nullptr;
    }

    CopyTo(clone);
    return clone;
}

void TiXmlDocument::Print(FILE* cfile, int depth) const
{
    assert(cfile);
    for (const TiXmlNode* node = FirstChild(); node; node = node->NextSibling())
    {
        node->Print(cfile, depth);
        fprintf(cfile, "\n");
    }
}

auto TiXmlDocument::Accept(TiXmlVisitor* visitor) const -> bool
{
    if (visitor->VisitEnter(*this))
    {
        for (const TiXmlNode* node = FirstChild(); node;
             node                  = node->NextSibling())
        {
            if (!node->Accept(visitor))
            {
                break;
            }
        }
    }
    return visitor->VisitExit(*this);
}

auto TiXmlAttribute::Next() const -> const TiXmlAttribute*
{
    // We are using knowledge of the sentinel. The sentinel
    // have a value or name.
    if (next->value.empty() && next->name.empty())
    {
        return nullptr;
    }
    return next;
}

/*
TiXmlAttribute* TiXmlAttribute::Next()
{
    // We are using knowledge of the sentinel. The sentinel
    // have a value or name.
    if ( next->value.empty() && next->name.empty() )
        return 0;
    return next;
}
*/

auto TiXmlAttribute::Previous() const -> const TiXmlAttribute*
{
    // We are using knowledge of the sentinel. The sentinel
    // have a value or name.
    if (prev->value.empty() && prev->name.empty())
    {
        return nullptr;
    }
    return prev;
}

/*
TiXmlAttribute* TiXmlAttribute::Previous()
{
    // We are using knowledge of the sentinel. The sentinel
    // have a value or name.
    if ( prev->value.empty() && prev->name.empty() )
        return 0;
    return prev;
}
*/

void TiXmlAttribute::Print(FILE* cfile, int /*depth*/, TIXML_STRING* str) const
{
    TIXML_STRING n, v;

    EncodeString(name, &n);
    EncodeString(value, &v);

    if (value.find('\"') == TIXML_STRING::npos)
    {
        if (cfile)
        {
            fprintf(cfile, "%s=\"%s\"", n.c_str(), v.c_str());
        }
        if (str)
        {
            (*str) += n;
            (*str) += "=\"";
            (*str) += v;
            (*str) += "\"";
        }
    }
    else
    {
        if (cfile)
        {
            fprintf(cfile, "%s='%s'", n.c_str(), v.c_str());
        }
        if (str)
        {
            (*str) += n;
            (*str) += "='";
            (*str) += v;
            (*str) += "'";
        }
    }
}

auto TiXmlAttribute::QueryIntValue(int* ival) const -> int
{
    if (TIXML_SSCANF(value.c_str(), "%d", ival) == 1)
    {
        return TIXML_SUCCESS;
    }
    return TIXML_WRONG_TYPE;
}

auto TiXmlAttribute::QueryDoubleValue(double* dval) const -> int
{
    if (TIXML_SSCANF(value.c_str(), "%lf", dval) == 1)
    {
        return TIXML_SUCCESS;
    }
    return TIXML_WRONG_TYPE;
}

void TiXmlAttribute::SetIntValue(int _value)
{
    char buf[64];
#if defined(TIXML_SNPRINTF)
    TIXML_SNPRINTF(buf, sizeof(buf), "%d", _value);
#else
    sprintf(buf, "%d", _value);
#endif
    SetValue(buf);
}

void TiXmlAttribute::SetDoubleValue(double _value)
{
    char buf[256];
#if defined(TIXML_SNPRINTF)
    TIXML_SNPRINTF(buf, sizeof(buf), "%g", _value);
#else
    sprintf(buf, "%g", _value);
#endif
    SetValue(buf);
}

auto TiXmlAttribute::IntValue() const -> int
{
    return atoi(value.c_str());
}

auto TiXmlAttribute::DoubleValue() const -> double
{
    return atof(value.c_str());
}

TiXmlComment::TiXmlComment(const TiXmlComment& copy) :
    TiXmlNode(TiXmlNode::TINYXML_COMMENT)
{
    copy.CopyTo(this);
}

auto TiXmlComment::operator=(const TiXmlComment& base) -> TiXmlComment&
{
    Clear();
    base.CopyTo(this);
    return *this;
}

void TiXmlComment::Print(FILE* cfile, int depth) const
{
    assert(cfile);
    for (int i = 0; i < depth; i++)
    {
        fprintf(cfile, "    ");
    }
    fprintf(cfile, "<!--%s-->", value.c_str());
}

void TiXmlComment::CopyTo(TiXmlComment* target) const
{
    TiXmlNode::CopyTo(target);
}

auto TiXmlComment::Accept(TiXmlVisitor* visitor) const -> bool
{
    return visitor->Visit(*this);
}

auto TiXmlComment::Clone() const -> TiXmlNode*
{
    auto* clone = new TiXmlComment();

    if (!clone)
    {
        return nullptr;
    }

    CopyTo(clone);
    return clone;
}

void TiXmlText::Print(FILE* cfile, int depth) const
{
    assert(cfile);
    if (cdata)
    {
        int i;
        fprintf(cfile, "\n");
        for (i = 0; i < depth; i++)
        {
            fprintf(cfile, "    ");
        }
        fprintf(cfile, "<![CDATA[%s]]>\n", value.c_str()); // unformatted output
    }
    else
    {
        TIXML_STRING buffer;
        EncodeString(value, &buffer);
        fprintf(cfile, "%s", buffer.c_str());
    }
}

void TiXmlText::CopyTo(TiXmlText* target) const
{
    TiXmlNode::CopyTo(target);
    target->cdata = cdata;
}

auto TiXmlText::Accept(TiXmlVisitor* visitor) const -> bool
{
    return visitor->Visit(*this);
}

auto TiXmlText::Clone() const -> TiXmlNode*
{
    TiXmlText* clone = nullptr;
    clone            = new TiXmlText("");

    if (!clone)
    {
        return nullptr;
    }

    CopyTo(clone);
    return clone;
}

TiXmlDeclaration::TiXmlDeclaration(
    const char* _version,
    const char* _encoding,
    const char* _standalone) :
    TiXmlNode(TiXmlNode::TINYXML_DECLARATION)
{
    version    = _version;
    encoding   = _encoding;
    standalone = _standalone;
}

#ifdef TIXML_USE_STL
TiXmlDeclaration::TiXmlDeclaration(
    const std::string& _version,
    const std::string& _encoding,
    const std::string& _standalone) :
    TiXmlNode(TiXmlNode::TINYXML_DECLARATION)
{
    version    = _version;
    encoding   = _encoding;
    standalone = _standalone;
}
#endif

TiXmlDeclaration::TiXmlDeclaration(const TiXmlDeclaration& copy) :
    TiXmlNode(TiXmlNode::TINYXML_DECLARATION)
{
    copy.CopyTo(this);
}

auto TiXmlDeclaration::operator=(const TiXmlDeclaration& copy)
    -> TiXmlDeclaration&
{
    Clear();
    copy.CopyTo(this);
    return *this;
}

void TiXmlDeclaration::Print(FILE* cfile, int /*depth*/, TIXML_STRING* str)
    const
{
    if (cfile)
    {
        fprintf(cfile, "<?xml ");
    }
    if (str)
    {
        (*str) += "<?xml ";
    }

    if (!version.empty())
    {
        if (cfile)
        {
            fprintf(cfile, "version=\"%s\" ", version.c_str());
        }
        if (str)
        {
            (*str) += "version=\"";
            (*str) += version;
            (*str) += "\" ";
        }
    }
    if (!encoding.empty())
    {
        if (cfile)
        {
            fprintf(cfile, "encoding=\"%s\" ", encoding.c_str());
        }
        if (str)
        {
            (*str) += "encoding=\"";
            (*str) += encoding;
            (*str) += "\" ";
        }
    }
    if (!standalone.empty())
    {
        if (cfile)
        {
            fprintf(cfile, "standalone=\"%s\" ", standalone.c_str());
        }
        if (str)
        {
            (*str) += "standalone=\"";
            (*str) += standalone;
            (*str) += "\" ";
        }
    }
    if (cfile)
    {
        fprintf(cfile, "?>");
    }
    if (str)
    {
        (*str) += "?>";
    }
}

void TiXmlDeclaration::CopyTo(TiXmlDeclaration* target) const
{
    TiXmlNode::CopyTo(target);

    target->version    = version;
    target->encoding   = encoding;
    target->standalone = standalone;
}

auto TiXmlDeclaration::Accept(TiXmlVisitor* visitor) const -> bool
{
    return visitor->Visit(*this);
}

auto TiXmlDeclaration::Clone() const -> TiXmlNode*
{
    auto* clone = new TiXmlDeclaration();

    if (!clone)
    {
        return nullptr;
    }

    CopyTo(clone);
    return clone;
}

void TiXmlUnknown::Print(FILE* cfile, int depth) const
{
    for (int i = 0; i < depth; i++)
    {
        fprintf(cfile, "    ");
    }
    fprintf(cfile, "<%s>", value.c_str());
}

void TiXmlUnknown::CopyTo(TiXmlUnknown* target) const
{
    TiXmlNode::CopyTo(target);
}

auto TiXmlUnknown::Accept(TiXmlVisitor* visitor) const -> bool
{
    return visitor->Visit(*this);
}

auto TiXmlUnknown::Clone() const -> TiXmlNode*
{
    auto* clone = new TiXmlUnknown();

    if (!clone)
    {
        return nullptr;
    }

    CopyTo(clone);
    return clone;
}

TiXmlAttributeSet::TiXmlAttributeSet()
{
    sentinel.next = &sentinel;
    sentinel.prev = &sentinel;
}

TiXmlAttributeSet::~TiXmlAttributeSet()
{
    assert(sentinel.next == &sentinel);
    assert(sentinel.prev == &sentinel);
}

void TiXmlAttributeSet::Add(TiXmlAttribute* addMe)
{
#ifdef TIXML_USE_STL
    assert(!Find(TIXML_STRING(
        addMe->Name()))); // Shouldn't be multiply adding to the set.
#else
    assert(!Find(addMe->Name())); // Shouldn't be multiply adding to the set.
#endif

    addMe->next = &sentinel;
    addMe->prev = sentinel.prev;

    sentinel.prev->next = addMe;
    sentinel.prev       = addMe;
}

void TiXmlAttributeSet::Remove(TiXmlAttribute* removeMe)
{
    TiXmlAttribute* node;

    for (node = sentinel.next; node != &sentinel; node = node->next)
    {
        if (node == removeMe)
        {
            node->prev->next = node->next;
            node->next->prev = node->prev;
            node->next       = nullptr;
            node->prev       = nullptr;
            return;
        }
    }
    assert(0); // we tried to remove a non-linked attribute.
}

#ifdef TIXML_USE_STL
auto TiXmlAttributeSet::Find(const std::string& name) const -> TiXmlAttribute*
{
    for (TiXmlAttribute* node = sentinel.next; node != &sentinel;
         node                 = node->next)
    {
        if (node->name == name)
        {
            return node;
        }
    }
    return nullptr;
}

auto TiXmlAttributeSet::FindOrCreate(const std::string& _name)
    -> TiXmlAttribute*
{
    TiXmlAttribute* attrib = Find(_name);
    if (!attrib)
    {
        attrib = new TiXmlAttribute();
        Add(attrib);
        attrib->SetName(_name);
    }
    return attrib;
}
#endif

auto TiXmlAttributeSet::Find(const char* name) const -> TiXmlAttribute*
{
    for (TiXmlAttribute* node = sentinel.next; node != &sentinel;
         node                 = node->next)
    {
        if (strcmp(node->name.c_str(), name) == 0)
        {
            return node;
        }
    }
    return nullptr;
}

auto TiXmlAttributeSet::FindOrCreate(const char* _name) -> TiXmlAttribute*
{
    TiXmlAttribute* attrib = Find(_name);
    if (!attrib)
    {
        attrib = new TiXmlAttribute();
        Add(attrib);
        attrib->SetName(_name);
    }
    return attrib;
}

#ifdef TIXML_USE_STL
auto operator>>(std::istream& in, TiXmlNode& base) -> std::istream&
{
    TIXML_STRING tag;
    tag.reserve(8 * 1000);
    base.StreamIn(&in, &tag);

    base.Parse(tag.c_str(), nullptr, TIXML_DEFAULT_ENCODING);
    return in;
}
#endif

#ifdef TIXML_USE_STL
auto operator<<(std::ostream& out, const TiXmlNode& base) -> std::ostream&
{
    TiXmlPrinter printer;
    printer.SetStreamPrinting();
    base.Accept(&printer);
    out << printer.Str();

    return out;
}

auto operator<<(std::string& out, const TiXmlNode& base) -> std::string&
{
    TiXmlPrinter printer;
    printer.SetStreamPrinting();
    base.Accept(&printer);
    out.append(printer.Str());

    return out;
}
#endif

auto TiXmlHandle::FirstChild() const -> TiXmlHandle
{
    if (node)
    {
        TiXmlNode* child = node->FirstChild();
        if (child)
        {
            return TiXmlHandle(child);
        }
    }
    return TiXmlHandle(nullptr);
}

auto TiXmlHandle::FirstChild(const char* value) const -> TiXmlHandle
{
    if (node)
    {
        TiXmlNode* child = node->FirstChild(value);
        if (child)
        {
            return TiXmlHandle(child);
        }
    }
    return TiXmlHandle(nullptr);
}

auto TiXmlHandle::FirstChildElement() const -> TiXmlHandle
{
    if (node)
    {
        TiXmlElement* child = node->FirstChildElement();
        if (child)
        {
            return TiXmlHandle(child);
        }
    }
    return TiXmlHandle(nullptr);
}

auto TiXmlHandle::FirstChildElement(const char* value) const -> TiXmlHandle
{
    if (node)
    {
        TiXmlElement* child = node->FirstChildElement(value);
        if (child)
        {
            return TiXmlHandle(child);
        }
    }
    return TiXmlHandle(nullptr);
}

auto TiXmlHandle::Child(int count) const -> TiXmlHandle
{
    if (node)
    {
        int i;
        TiXmlNode* child = node->FirstChild();
        for (i = 0; child && i < count; child = child->NextSibling(), ++i)
        {
            // nothing
        }
        if (child)
        {
            return TiXmlHandle(child);
        }
    }
    return TiXmlHandle(nullptr);
}

auto TiXmlHandle::Child(const char* value, int count) const -> TiXmlHandle
{
    if (node)
    {
        int i;
        TiXmlNode* child = node->FirstChild(value);
        for (i = 0; child && i < count; child = child->NextSibling(value), ++i)
        {
            // nothing
        }
        if (child)
        {
            return TiXmlHandle(child);
        }
    }
    return TiXmlHandle(nullptr);
}

auto TiXmlHandle::ChildElement(int count) const -> TiXmlHandle
{
    if (node)
    {
        int i;
        TiXmlElement* child = node->FirstChildElement();
        for (i     = 0; child && i < count;
             child = child->NextSiblingElement(), ++i)
        {
            // nothing
        }
        if (child)
        {
            return TiXmlHandle(child);
        }
    }
    return TiXmlHandle(nullptr);
}

auto TiXmlHandle::ChildElement(const char* value, int count) const
    -> TiXmlHandle
{
    if (node)
    {
        int i;
        TiXmlElement* child = node->FirstChildElement(value);
        for (i     = 0; child && i < count;
             child = child->NextSiblingElement(value), ++i)
        {
            // nothing
        }
        if (child)
        {
            return TiXmlHandle(child);
        }
    }
    return TiXmlHandle(nullptr);
}

auto TiXmlPrinter::VisitEnter(const TiXmlDocument&) -> bool
{
    return true;
}

auto TiXmlPrinter::VisitExit(const TiXmlDocument&) -> bool
{
    return true;
}

auto TiXmlPrinter::VisitEnter(
    const TiXmlElement& element,
    const TiXmlAttribute* firstAttribute) -> bool
{
    DoIndent();
    buffer += "<";
    buffer += element.Value();

    for (const TiXmlAttribute* attrib = firstAttribute; attrib;
         attrib                       = attrib->Next())
    {
        buffer += " ";
        attrib->Print(nullptr, 0, &buffer);
    }

    if (!element.FirstChild())
    {
        buffer += " />";
        DoLineBreak();
    }
    else
    {
        buffer += ">";
        if (element.FirstChild()->ToText()
            && element.LastChild() == element.FirstChild()
            && element.FirstChild()->ToText()->CDATA() == false)
        {
            simpleTextPrint = true;
            // no DoLineBreak()!
        }
        else
        {
            DoLineBreak();
        }
    }
    ++depth;
    return true;
}

auto TiXmlPrinter::VisitExit(const TiXmlElement& element) -> bool
{
    --depth;
    if (!element.FirstChild())
    {
        // nothing.
    }
    else
    {
        if (simpleTextPrint)
        {
            simpleTextPrint = false;
        }
        else
        {
            DoIndent();
        }
        buffer += "</";
        buffer += element.Value();
        buffer += ">";
        DoLineBreak();
    }
    return true;
}

auto TiXmlPrinter::Visit(const TiXmlText& text) -> bool
{
    if (text.CDATA())
    {
        DoIndent();
        buffer += "<![CDATA[";
        buffer += text.Value();
        buffer += "]]>";
        DoLineBreak();
    }
    else if (simpleTextPrint)
    {
        TIXML_STRING str;
        TiXmlBase::EncodeString(text.ValueTStr(), &str);
        buffer += str;
    }
    else
    {
        DoIndent();
        TIXML_STRING str;
        TiXmlBase::EncodeString(text.ValueTStr(), &str);
        buffer += str;
        DoLineBreak();
    }
    return true;
}

auto TiXmlPrinter::Visit(const TiXmlDeclaration& declaration) -> bool
{
    DoIndent();
    declaration.Print(nullptr, 0, &buffer);
    DoLineBreak();
    return true;
}

auto TiXmlPrinter::Visit(const TiXmlComment& comment) -> bool
{
    DoIndent();
    buffer += "<!--";
    buffer += comment.Value();
    buffer += "-->";
    DoLineBreak();
    return true;
}

auto TiXmlPrinter::Visit(const TiXmlUnknown& unknown) -> bool
{
    DoIndent();
    buffer += "<";
    buffer += unknown.Value();
    buffer += ">";
    DoLineBreak();
    return true;
}
