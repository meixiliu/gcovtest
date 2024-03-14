#include <limits.h>
#include "googletest/include/gtest/gtest.h"
#include "googletest/include/gmock/gmock.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <sstream>
#include <random>

#include "tinyxml2/tinyxml2.h"

using namespace tinyxml2;

using namespace testing;


TEST(TEST_StrPair, SetStr_TransferTo)
{
    StrPair A, B;
    A.Set(NULL, NULL, 0);
    static const char *tmp = "<this>";
    A.SetStr(tmp, 0);
    A.TransferTo(&A);
    A.TransferTo(&B);
    EXPECT_STREQ(tmp, B.GetStr());
}

TEST(TEST_StrPair, Set_Reset)
{
    StrPair A, B;
    A.Set(NULL, NULL, StrPair::COMMENT);
    A.Reset();

    char * tmp = new char[10];
    A.Set(tmp, NULL, 0x200); // NEEDS_DELETE
    A.Reset();
    EXPECT_EQ(NULL, A.GetStr());

    A.Set(NULL, NULL, 0x200); // Delete Null
    A.Reset();
    EXPECT_EQ(NULL, A.GetStr());

}

TEST(TEST_StrPair, GetStr_CollapseWhitespace)
{

    StrPair A;
    char *tmp = new char[12];
    std::sprintf(tmp, "\n<a>\n</a>\n");
    A.Set(tmp, &tmp[11], StrPair::NEEDS_WHITESPACE_COLLAPSING);
    EXPECT_STREQ("<a> </a>", A.GetStr());
    delete [] tmp;

    tmp = new char[12];
    std::sprintf(tmp, "\n\n");     // all \n
    A.Set(tmp, &tmp[12], StrPair::NEEDS_WHITESPACE_COLLAPSING);
    EXPECT_STREQ("", A.GetStr());
    delete [] tmp;

    tmp = new char[12];
    std::sprintf(tmp, "\r\n\r<a>\r\n</a>\r\n");
    A.Set(tmp, &tmp[12], StrPair::NEEDS_WHITESPACE_COLLAPSING | StrPair::NEEDS_NEWLINE_NORMALIZATION);
    EXPECT_STREQ("<a> </a>", A.GetStr());
    delete [] tmp;

    tmp = new char[12];
    std::sprintf(tmp, "\n\r<a>\n</a>\r\n");
    A.Set(tmp, &tmp[12], StrPair::NEEDS_WHITESPACE_COLLAPSING | StrPair::NEEDS_NEWLINE_NORMALIZATION);
    EXPECT_STREQ("<a> </a>", A.GetStr());
    delete [] tmp;

    tmp = new char[50];
    std::sprintf(tmp, " &#x4e2d;  &#x4e2d &quot; &qut; &quot.");
    A.Set(tmp, &tmp[50], StrPair::NEEDS_ENTITY_PROCESSING);
    std::cout << A.GetStr() << std::endl;
//    EXPECT_STREQ("<a> </a>", A.GetStr());
    delete [] tmp;

    tmp = new char[50];
    std::sprintf(tmp, "<Hi>");
    A.Set(tmp, &tmp[50], 0); // no flag
    std::cout << A.GetStr() << std::endl;
    EXPECT_STREQ("<Hi>", A.GetStr());
    delete [] tmp;
}

TEST(TEST_StrPair, ParseName)
{

    StrPair A;
    char *tmp = new char[12];
    std::sprintf(tmp, "<My-Name></My-Name>");
    char* res = A.ParseName(tmp);
    EXPECT_EQ(0, res);

    res = A.ParseName(0); // p == 0
    EXPECT_EQ(0, res);

    tmp[0] = 0;
    res = A.ParseName(tmp); // *p == 0
    EXPECT_EQ(0, res);

    std::sprintf(tmp, "My-Name></My-Name>");
    tmp[1] = 0;             // *(p+1) == 0
    res = A.ParseName(tmp);
    EXPECT_STREQ("", res);

    std::sprintf(tmp, "My-Name></My-Name>");
    res = A.ParseName(tmp);
    EXPECT_STREQ("></My-Name>", res);
    delete [] tmp;
}

TEST(TEST_StrPair, ParseText)
{

    StrPair A;
    char *tmp = new char[12];
    int lineP = 0;
    std::sprintf(tmp, "<My-N\namee></My-Name>");
    char* res = A.ParseText(tmp, "e>", 0, &lineP);
    EXPECT_STREQ("</My-Name>", res);
    EXPECT_EQ(1, lineP);
    tmp[0] = 0;
    res = A.ParseText(tmp, ">", 0, &lineP);
    EXPECT_EQ(0, res);
    delete [] tmp;
}

TEST(TEST_XMLUtil, SetBoolSerialization)
{
    XMLUtil::SetBoolSerialization("T", "F");
    XMLUtil::SetBoolSerialization(NULL, NULL);
}

TEST(TEST_XMLUtil, ReadBOM)
{
    char *tmp = new char[5];
    tmp[0] = 0xefU; tmp[1] = 0xbbU; tmp[2] = 0xbfU; tmp[3] = 'A'; tmp[4] = 0;
    bool bom = 0;
    const char* res = XMLUtil::ReadBOM(tmp, &bom);
    EXPECT_STREQ("A", res);

    tmp[0] = 0xafU; tmp[1] = 0xabU; tmp[2] = 0xbfU; tmp[3] = 'A'; tmp[4] = 0;
    bom = 0;
    res = XMLUtil::ReadBOM(tmp, &bom);
    EXPECT_STREQ(tmp, res);

    tmp[0] = 0xefU; tmp[1] = 0xabU; tmp[2] = 0xbfU; tmp[3] = 'A'; tmp[4] = 0;
    bom = 0;
    res = XMLUtil::ReadBOM(tmp, &bom);
    EXPECT_STREQ(tmp, res);

    tmp[0] = 0xefU; tmp[1] = 0xbbU; tmp[2] = 0xafU; tmp[3] = 'A'; tmp[4] = 0;
    bom = 0;
    res = XMLUtil::ReadBOM(tmp, &bom);
    EXPECT_STREQ(tmp, res);

    delete [] tmp;
}

TEST(TEST_XMLUtil, ConvertUTF32ToUTF8)
{
    unsigned long input = 0;
    char* output = new char[4];
    int length = 0;

    // 1 字节
    input = 0x80 - 23;
    XMLUtil::ConvertUTF32ToUTF8(input, output, &length);
    std::printf(">>>>> [%s]\n", output);
//    EXPECT_STREQ("i", output);

    // 2 字节
    input = 0x800 - 23;
    XMLUtil::ConvertUTF32ToUTF8(input, output, &length);
    std::printf(">>>>> [%s]\n", output);
//    EXPECT_STREQ("\337\251", output);

    // 3 字节
    input = 0x10000 - 23;
    XMLUtil::ConvertUTF32ToUTF8(input, output, &length);
    std::printf(">>>>> [%s]\n", output);
//    EXPECT_STREQ("\357\277\251", output);

    // 4 字节
    input = 0x200000 - 0x100000;
    XMLUtil::ConvertUTF32ToUTF8(input, output, &length);
    std::printf(">>>>> [%s]\n", output);
//    EXPECT_STREQ("\364\200\200\200", output);

    // 长 字节
    input = 0x300000;
    XMLUtil::ConvertUTF32ToUTF8(input, output, &length);
    std::printf(">>>>> [%s]\n", output);
//    EXPECT_STREQ("\364\200\200\200", output);
    // todo: 这里存在死代码，switch的default分支测不到

    delete [] output;
}

TEST(TEST_XMLUtil, GetCharacterRef)
{
    const char* res = NULL;
    char* str = NULL;
    int length = 0;
    str = new char[40];
    memset(str, 0, sizeof(char)*30);

    char P[24][30] = {
            "&#x4e2d; adf",     " adf",             // line: 533
            "& x4e2d; adf",     " x4e2d; adf",      // line: 490
            "&#",               "#",                // line: 483
            "&#x",              "-",                // line: 493
            "&#x4e2d adf",      "-",                // line: 500
            "&#xAe2D; adf",     " adf",             // line: 516
            "&#x\47e2D; adf",   "-",                // line: 510
            "&#4e2d; adf",      "-",                // line: 533
            "&#4e2d adf",       "-",                // todo: line: 536 死代码，*(p + 2) 不可能为 0
            "&#4e2#; adf",      " adf",             // line: 549 -
            "&#\47 4323; ",     "-",                // line: 550 -
            "&#4323; adf",      " adf",             // all
    };

    for(int i = 0; i < 24; i += 2)
    {
        res = const_cast<char*>(XMLUtil::GetCharacterRef(P[i], str, &length));

//        std::printf(">>> [%s] [%s]\n", str, res);
        if(P[i+1][0] == '-')
            EXPECT_STREQ(NULL, res);
        else
            EXPECT_STREQ(P[i+1], res);
        memset(str, 0, sizeof(char)*40);
    }

    delete [] str;

}



TEST(TEST_XMLUtil, ToStr)
{
    char buf[50];
    XMLUtil::ToStr((int)10, buf, 50);
    EXPECT_STREQ("10", buf);
    XMLUtil::ToStr((unsigned)10, buf, 50);
    EXPECT_STREQ("10", buf);
    XMLUtil::ToStr((bool)1, buf, 50);
    EXPECT_STREQ("true", buf);
    XMLUtil::ToStr((bool)0, buf, 50);
    EXPECT_STREQ("false", buf);
    XMLUtil::ToStr((float)1.123123, buf, 50);
    EXPECT_STREQ("1.123123", buf);
    XMLUtil::ToStr((double)3.1234123123, buf, 50);
    EXPECT_STREQ("3.1234123123000002", buf);
    XMLUtil::ToStr((int64_t)10, buf, 50);
    EXPECT_STREQ("10", buf);
    XMLUtil::ToStr((uint64_t)10, buf, 50);
    EXPECT_STREQ("10", buf);
}

TEST(TEST_XMLUtil, SkipWhiteSpace_IsWhiteSpace_IsNameStartChar_IsNameChar_StringEqual)
{
    char buf[50];
    int line = 0;
    char* res;

    EXPECT_EQ(0, XMLUtil::IsWhiteSpace('\332'));
    EXPECT_EQ(1, XMLUtil::IsWhiteSpace('\n'));

    EXPECT_EQ(1, XMLUtil::IsNameStartChar((char)200));
    EXPECT_EQ(1, XMLUtil::IsNameStartChar('a'));
    EXPECT_EQ(1, XMLUtil::IsNameStartChar(':'));
    EXPECT_EQ(1, XMLUtil::IsNameStartChar('_'));
    EXPECT_EQ(0, XMLUtil::IsNameStartChar('4'));

    EXPECT_EQ(1, XMLUtil::IsNameChar('a'));
    EXPECT_EQ(1, XMLUtil::IsNameChar('.'));
    EXPECT_EQ(1, XMLUtil::IsNameChar('-'));
    EXPECT_EQ(1, XMLUtil::IsNameChar('4'));
    EXPECT_EQ(0, XMLUtil::IsNameChar('@'));

    sprintf(buf, "\n\n  adfasd");
    res = XMLUtil::SkipWhiteSpace(buf, &line);
    std::printf(">>>>>[%s]\n", res);
    EXPECT_STREQ(res, "adfasd");
    sprintf(buf, "\n\n  adfasd");
    res = XMLUtil::SkipWhiteSpace(buf, NULL);
    std::printf(">>>>>[%s]\n", res);
    EXPECT_STREQ(res, "adfasd");

    EXPECT_EQ(XMLUtil::StringEqual(buf, buf, 10), 1);
    EXPECT_EQ(XMLUtil::StringEqual("abcd", "abcde", 4), 1);
    EXPECT_EQ(XMLUtil::StringEqual("hasdf", "abcd", 10), 0);
}

TEST(TEST_XMLUtil, IsPrefixHex)
{
    char buf[50];
    bool res;
    sprintf(buf, "\n");
    res = XMLUtil::IsPrefixHex(buf);
    EXPECT_EQ(res, 0);
    sprintf(buf, "0");
    res = XMLUtil::IsPrefixHex(buf);
    EXPECT_EQ(res, 0);
    sprintf(buf, "0x");
    res = XMLUtil::IsPrefixHex(buf);
    EXPECT_EQ(res, 1);
    sprintf(buf, "0X");
    res = XMLUtil::IsPrefixHex(buf);
    EXPECT_EQ(res, 1);
    sprintf(buf, "ab");
    res = XMLUtil::IsPrefixHex(buf);
    EXPECT_EQ(res, 0);
}

TEST(TEST_XMLUtil, ToInt_ToUnsigned_ToBool_ToFloat_ToDouble_ToInt64_ToUnsigned64)
{
    char buf[50];
    int x1 = 0;
    sprintf(buf, "10"); XMLUtil::ToInt(buf, &x1);
    EXPECT_EQ(10, x1);
    sprintf(buf, "0x10"); XMLUtil::ToInt(buf, &x1);
    EXPECT_EQ(16, x1);
    sprintf(buf, "avs"); XMLUtil::ToInt(buf, &x1);
    EXPECT_EQ(16, x1);

    unsigned x2 = 0;
    sprintf(buf, "10"); XMLUtil::ToUnsigned(buf, &x2);
    EXPECT_EQ(10, x2);
    sprintf(buf, "0x10"); XMLUtil::ToUnsigned(buf, &x2);
    EXPECT_EQ(16, x2);
    sprintf(buf, "avs"); XMLUtil::ToUnsigned(buf, &x2);
    EXPECT_EQ(16, x2);

    bool x3 = 0;
    sprintf(buf, "10"); XMLUtil::ToBool(buf, &x3);
    EXPECT_EQ(1, x3);
    sprintf(buf, "0x0"); XMLUtil::ToBool(buf, &x3);
    EXPECT_EQ(0, x3);
    sprintf(buf, "akde"); XMLUtil::ToBool(buf, &x3);
    EXPECT_EQ(0, x3);
    sprintf(buf, "false"); XMLUtil::ToBool(buf, &x3);
    EXPECT_EQ(0, x3);
    sprintf(buf, "True"); XMLUtil::ToBool(buf, &x3);
    EXPECT_EQ(1, x3);

    float x4 = 0;
    sprintf(buf, "10.5"); XMLUtil::ToFloat(buf, &x4);
    EXPECT_EQ(10.5, x4);
    sprintf(buf, "qqwef"); XMLUtil::ToFloat(buf, &x4);
    EXPECT_EQ(10.5, x4);


    double x5 = 0;
    sprintf(buf, "10.5"); XMLUtil::ToDouble(buf, &x5);
    EXPECT_EQ(10.5, x5);
    sprintf(buf, "qqwef"); XMLUtil::ToDouble(buf, &x5);
    EXPECT_EQ(10.5, x5);

    int64_t x6 = 0;
    sprintf(buf, "10"); XMLUtil::ToInt64(buf, &x6);
    EXPECT_EQ(10, x6);
    sprintf(buf, "0x10"); XMLUtil::ToInt64(buf, &x6);
    EXPECT_EQ(16, x6);
    sprintf(buf, "avs"); XMLUtil::ToInt64(buf, &x6);
    EXPECT_EQ(16, x6);

    uint64_t x7 = 0;
    sprintf(buf, "10"); XMLUtil::ToUnsigned64(buf, &x7);
    EXPECT_EQ(10, x7);
    sprintf(buf, "0x10"); XMLUtil::ToUnsigned64(buf, &x7);
    EXPECT_EQ(16, x7);
    sprintf(buf, "avs"); XMLUtil::ToUnsigned64(buf, &x7);
    EXPECT_EQ(16, x7);
    sprintf(buf, "\njo\nk\n"); XMLUtil::ToUnsigned64(buf, &x7); // 奇怪的行为
    EXPECT_EQ(16, x7);
}

TEST(TEST_MemPool, MemPool)
{
    int res = 0;
    MemPoolT< sizeof(int) > intPool;
    res = intPool.ItemSize();
    EXPECT_EQ(sizeof(int), res);

    intPool.Free(NULL);
}

TEST(TEST_XMLNode, XMLNode)
{
    XMLDocument doc;
    doc.LoadFile( "./testxml/test.xml" );

    static const char* xml =
            "<?xml version=\"1.0\"?>"
            "<!DOCTYPE PLAY SYSTEM \"play.dtd\">"
            "<PLAY>"
            "<TITLE>A Midsummer Night's Dream</TITLE>"
            "</PLAY>"
            "<Hello/>";

    XMLDocument doc2;
    doc2.Parse( xml );

    XMLDocument doc3;

    doc3.LoadFile( "./testxml/overflow.xml" );

    XMLNode *a = &doc;
    const char *res = a->Value();

    XMLNode* fe = doc.FirstChild();
    XMLNode* fe2 = doc.FirstChild()->NextSibling();
    XMLNode* fe3 = doc.FirstChild()->NextSibling()->NextSibling();

    fe->SetValue("fuck\n", 1);
    std::printf(">>>> [%s]\n", fe->Value());

    char k[10];
    std::sprintf(k, "my name");
    fe->SetValue(k, 0);
    std::printf(">>>> [%s]\n", fe->Value());

    XMLNode *cl = a->DeepClone(&doc);
    cl = fe->DeepClone(&doc);
    cl = fe->DeepClone(&doc2);

    // --------- InsertEndChild ----------- //
    fe->InsertEndChild(doc2.FirstChild());

    // --------- InsertFirstChild ----------- //
    fe->InsertFirstChild(doc2.FirstChild());
    XMLText*    theText = doc.GetDocument()->NewText( "Hello" );
    fe -> InsertFirstChild( theText );
    theText = doc.GetDocument()->NewText( "Hello!" );
    fe3 -> InsertFirstChild( theText );


    // --------- InsertFirstChild ----------- //
    theText = doc.GetDocument()->NewText( "Hello!" );
    fe2 -> InsertAfterChild( fe->FirstChild() ,theText );
    theText = doc.GetDocument()->NewText( "Hello!" );
    fe2 -> InsertAfterChild(doc2.FirstChild(), theText);
    theText = doc2.GetDocument()->NewText( "Hello!" );
    fe2 -> InsertAfterChild( fe->FirstChild(), theText );
    theText = doc.GetDocument()->NewText( "Hello!" );
    fe2 -> InsertAfterChild( fe2->FirstChild(), fe2->FirstChild() );
    theText = doc.GetDocument()->NewText( "Hello!" );
    fe2 -> InsertAfterChild( fe2->FirstChild(), theText );
    theText = doc.GetDocument()->NewText( "Hello!" );
    fe2 -> InsertAfterChild( fe2->FirstChild()->NextSibling()->NextSibling()->NextSibling(), theText );

    const XMLElement *el = NULL;

    // --------- FirstChildElement ----------- //
    el = fe2 -> FirstChildElement("aname");
    el = fe2 -> FirstChildElement("English");


    // --------- LastChildElement ----------- //
    el = fe2 -> LastChildElement("aname");
    el = fe2 -> LastChildElement("div");

    // --------- NextSiblingElement ----------- //
    el = fe2 -> NextSiblingElement("aname");
    el = fe2 -> NextSiblingElement("fuck");

    // --------- PreviousSiblingElement ----------- //
    el = fe2 -> PreviousSiblingElement("Englisha");
    el = fe2 -> PreviousSiblingElement("my name");

    // --------- DeepParse ----------- //
    XMLDocument parsexml;
    parsexml.Parse("<x>");
    parsexml.Parse("<x> ");
    parsexml.Parse("<x></y>");
    parsexml.Parse("<?xml version=\"1.0\" ?>");      // 1095
    parsexml.Parse("<?xml version=\"1.0\" ?><?xml hahaha=\"1.0\" ?>");      // 1095
    parsexml.Parse("<div></div><?xml version=\"1.0\" ?><?xml hahaha=\"1.0\" ?>");      // 1095
    parsexml.Parse("<div><?xml version=\"1.0\" ?><?xml hahaha=\"1.0\" ?></div>");      // 1095
    parsexml.Parse("<></>");      // 1095
    parsexml.Parse("123adsf</ae>");      // 1095

    parsexml.Parse(NULL);      // 1095


    // --------- InsertChildPreamble ----------- //
    XMLDocument doc4;
    const char* xml2 = "<?xml version=\"1.0\" ?>"
                      "<root>"
                      "<one>"
                      "<haha>"
                      "<b>element 1</b>text<!-- comment -->"
                      "</haha>"
                      "</one>"
                      "<two/>"
                      "</root>";
    doc4.Parse(xml2);
    XMLElement* haha = doc4.RootElement()->FirstChildElement("one")->FirstChildElement("haha");
    XMLElement* two = doc4.RootElement()->FirstChildElement("two");
    two->InsertFirstChild(haha);
    // --------- DeleteNode ----------- //
    EXPECT_DEATH(doc2.DeleteNode(NULL), "");
    // --------- Unlink ----------- //
    fe2 -> DeleteChild(fe2);

    // --------- ToXXX ----------- //
    EXPECT_EQ(fe, fe->ToElement());
    EXPECT_EQ(NULL, fe->ToDocument());
    EXPECT_EQ(NULL, fe->ToComment());
    EXPECT_EQ(NULL, fe->ToDeclaration());
    EXPECT_EQ(NULL, fe->ToText());
    EXPECT_EQ(NULL, fe->ToUnknown());

}

TEST(TEST_XMLText, XMLText)
{
    // --------- DeepParse ----------- //
    XMLDocument parsexml;
    parsexml.Parse("<![CDATA[asdfasdf]]>");
    parsexml.Parse("<![CDATA[asdfasdf]>");

    const char* xml = "<element>"
                      "<a> This \nis &apos;  text  &apos; </a>"
                      "<b>  This is &apos; text &apos;  \n</b>"
                      "<c>This  is  &apos;  \n\n text &apos;</c>"
                      "</element>";
    XMLDocument doc( true, COLLAPSE_WHITESPACE );
    doc.Parse( xml );

    XMLDocument doc2(false);
    doc2.Parse( xml );

    parsexml.Parse("![CDATA[asdfasdf]>");
    parsexml.Parse("<a>asdfasdf<");

    // --------- ShallowClone ----------- //
    XMLText* theText = doc.GetDocument()->NewText( "Hello!");
    XMLNode* p = theText ->ShallowClone(NULL);
    p = theText ->ShallowClone(&doc);

    // --------- ShallowEqual ----------- //
    theText -> ShallowEqual(p);
    theText -> ShallowEqual(&doc);
    theText -> ShallowEqual(doc.GetDocument()->NewText( "Hello"));

    // --------- Accept ----------- //
    doc.Print();
}

TEST(TEST_XMLComment, XMLComment)
{
// --------- DeepParse ----------- //
    XMLDocument doc;
    doc.Parse("<!-- DATA[asdfasdf]>");
    doc.Parse("<!-- asdfasdf -->");

    // --------- ShallowClone ----------- //
    XMLComment* theComment = doc.GetDocument() -> NewComment("Hello!");
    XMLNode* p = theComment ->ShallowClone(NULL);
    p = theComment ->ShallowClone(&doc);

    // --------- ShallowEqual ----------- //
    theComment -> ShallowEqual(p);
    theComment -> ShallowEqual(&doc);
    theComment -> ShallowEqual(doc.GetDocument()->NewComment( "Hello"));

    // --------- ToXXX ----------- //
    EXPECT_EQ(theComment, theComment->ToComment());


    // --------- Accept ----------- //
    doc.Print();
}

TEST(TEST_XMLDeclaration, XMLDeclaration)
{
    // --------- DeepParse ----------- //
    XMLDocument doc;
    doc.Parse("<?xml version=\"1.0\" adsf>");
    doc.Parse("<?xml version=\"1.0\" ?>");

    // --------- ShallowClone ----------- //
    XMLDeclaration* theDeclaration = doc.GetDocument() -> NewDeclaration("Hello!");
    XMLNode* p = theDeclaration ->ShallowClone(NULL);
    p = theDeclaration ->ShallowClone(&doc);

    // --------- ShallowEqual ----------- //
    theDeclaration -> ShallowEqual(p);
    theDeclaration -> ShallowEqual(&doc);
    theDeclaration -> ShallowEqual(doc.GetDocument()->NewDeclaration( "asdfasdflo"));

    // --------- Accept ----------- //
    doc.Print();
}

TEST(TEST_XMLUnknown, XMLUnknown)
{
    // --------- DeepParse ----------- //
    XMLDocument doc;
    doc.Parse("<#asdf>");
    doc.Parse("\n<root>\n<! \n");
    doc.Parse("<?xml version=\"1.0\" ?>\n"
        "<!DOCTYPE PLAY SYSTEM 'play.dtd'>\n"
        "<!ELEMENT title (#PCDATA)>\n"
        "<!ELEMENT books (title,authors)>\n"
        "<element/>");

    // --------- ShallowClone ----------- //
    XMLUnknown* theUnknown = doc.GetDocument() -> NewUnknown("Hello!");
    XMLNode* p = NULL;
    p = theUnknown ->ShallowClone(NULL);
    p = theUnknown ->ShallowClone(&doc);

    // --------- ShallowEqual ----------- //
    theUnknown -> ShallowEqual(p);
    theUnknown -> ShallowEqual(&doc);
    theUnknown -> ShallowEqual(doc.GetDocument()->NewUnknown( "asdfasdflo"));

    // --------- ShallowEqual ----------- //
    EXPECT_EQ(theUnknown, theUnknown->ToUnknown());

    // --------- Accept ----------- //
    doc.Print();
}


TEST(TEST_XMLAttribute, XMLAttribute)
{
    // --------- DeepParse ----------- //
    XMLDocument doc;
    doc.Parse("<div name ></div>");
    doc.Parse("<element att\0r='red' attr='blue' />");
    doc.Parse("<element attr=red />");
    doc.Parse("<element attr='red />");

    XMLDocument doc2(false);
    doc2.Parse("<div name='fuck' ></div>");

    // --------- QueryValue ----------- //

    const XMLAttribute* theAttribute = NULL;

    doc2.Parse("<div name='10' ></div>");
    theAttribute = doc2.RootElement()->FindAttribute("name");
    int aint = 0;
    theAttribute->QueryIntValue(&aint);
    EXPECT_EQ(aint, 10);
    doc2.Parse("<div name='10' ></div>");
    theAttribute = doc2.RootElement()->FindAttribute("name");
    int64_t aint64 = 0;
    theAttribute->QueryInt64Value(&aint64);
    EXPECT_EQ(aint64, 10);
    doc2.Parse("<div name='10' ></div>");
    theAttribute = doc2.RootElement()->FindAttribute("name");
    unsigned aunsigned = 0;
    theAttribute->QueryUnsignedValue(&aunsigned);
    EXPECT_EQ(aunsigned, 10);
    doc2.Parse("<div name='10' ></div>");
    theAttribute = doc2.RootElement()->FindAttribute("name");
    uint64_t aunsigned64 = 0;
    theAttribute->QueryUnsigned64Value(&aunsigned64);
    EXPECT_EQ(aunsigned64, 10);
    doc2.Parse("<div name='0' ></div>");
    theAttribute = doc2.RootElement()->FindAttribute("name");
    bool abool = 0;
    theAttribute->QueryBoolValue(&abool);
    EXPECT_EQ(abool, 0);
    doc2.Parse("<div name='1.5' ></div>");
    theAttribute = doc2.RootElement()->FindAttribute("name");
    float afloat = 0;
    theAttribute->QueryFloatValue(&afloat);
    EXPECT_EQ(afloat, 1.5);
    doc2.Parse("<div name='1.5' ></div>");
    theAttribute = doc2.RootElement()->FindAttribute("name");
    double adouble = 0;
    theAttribute->QueryDoubleValue(&adouble);
    EXPECT_EQ(adouble, 1.5);
    EXPECT_EQ(theAttribute->QueryDoubleValue(&adouble), 0);

    doc2.Parse("<div name='asdfa' ></div>");
    theAttribute = doc2.RootElement()->FindAttribute("name");
    theAttribute->QueryIntValue(&aint);
    theAttribute->QueryInt64Value(&aint64);
    theAttribute->QueryUnsignedValue(&aunsigned);
    theAttribute->QueryUnsigned64Value(&aunsigned64);
    theAttribute->QueryBoolValue(&abool);
    theAttribute->QueryFloatValue(&afloat);
    theAttribute->QueryDoubleValue(&adouble);
    EXPECT_EQ(theAttribute->QueryDoubleValue(&adouble), 2);



    doc2.Parse("<div name='asdfa' ></div>");
    theAttribute = doc2.RootElement()->FindAttribute("name");
    XMLElement* theElement = doc2.RootElement();

    theElement->SetAttribute("name", "Hello");
    EXPECT_STREQ(theElement->FindAttribute("name")->Value(), "Hello");

    theElement->SetAttribute("name", (int)10);
    EXPECT_STREQ(theElement->FindAttribute("name")->Value(), "10");

    theElement->SetAttribute("name", (unsigned)11);
    EXPECT_STREQ(theElement->FindAttribute("name")->Value(), "11");

    theElement->SetAttribute("name", (int64_t)12);
    EXPECT_STREQ(theElement->FindAttribute("name")->Value(), "12");

    theElement->SetAttribute("name", (int64_t)13);
    EXPECT_STREQ(theElement->FindAttribute("name")->Value(), "13");

    theElement->SetAttribute("name", (uint64_t)14);
    EXPECT_STREQ(theElement->FindAttribute("name")->Value(), "14");

    theElement->SetAttribute("name", (bool)0);
    EXPECT_STREQ(theElement->FindAttribute("name")->Value(), "false");

    theElement->SetAttribute("name", (float)1.5);
    EXPECT_STREQ(theElement->FindAttribute("name")->Value(), "1.5");

    theElement->SetAttribute("name", (double)2.5);
    EXPECT_STREQ(theElement->FindAttribute("name")->Value(), "2.5");

    theElement->QueryIntAttribute("nme", 0);
    theElement->QueryInt64Attribute("nme", 0);
    theElement->QueryUnsignedAttribute("nme", 0);
    theElement->QueryUnsigned64Attribute("nme", 0);
    theElement->QueryBoolAttribute("nme", 0);
    theElement->QueryFloatAttribute("nme", 0);
    theElement->QueryDoubleAttribute("nme", 0);
    theElement->QueryStringAttribute("nme", 0);
    EXPECT_EQ(theElement->QueryStringAttribute("nme", 0), 1);

    char* res = NULL;
    theElement->QueryStringAttribute("name", (const char**) &res);
    EXPECT_EQ(theElement->QueryStringAttribute("name", (const char**) &res), 0);


}

TEST(TEST_XMLElement, XMLElement)
{
    XMLDocument doc;
    XMLElement* theElement = NULL;
    int         aint = 0;
    unsigned    aunsigned = 0;
    int64_t     aint64 = 0;
    uint64_t    aunsigned64 = 0;
    bool        abool = 0;
    float       afloat = 0;
    double      adouble = 0;

// --------- Attribute ----------- //
    doc.Parse("<div name='abc' ></div>");
    theElement = doc.RootElement();
    EXPECT_STREQ(theElement->Attribute("name"), "abc");
    EXPECT_STREQ(theElement->Attribute("name", NULL), "abc");
    EXPECT_STREQ(theElement->Attribute("name", "abc"), "abc");
    EXPECT_STREQ(theElement->Attribute("name", "abcd"), NULL);

    // --------- XXXAttribute ----------- //
    doc.Parse("<div name='10' ></div>");
    theElement = doc.RootElement();

    aint = theElement->IntAttribute("name", 0);
    EXPECT_EQ(aint, 10);
    doc.Parse("<div name='10' ></div>");
    theElement = doc.RootElement();
    aint64 = theElement->Int64Attribute("name", 0);
    EXPECT_EQ(aint64, 10);
    doc.Parse("<div name='10' ></div>");
    theElement = doc.RootElement();
    aunsigned = theElement->UnsignedAttribute("name", 0);
    EXPECT_EQ(aunsigned, 10);
    doc.Parse("<div name='10' ></div>");
    theElement = doc.RootElement();
    aunsigned64 = theElement->Unsigned64Attribute("name", 0);
    EXPECT_EQ(aunsigned64, 10);
    doc.Parse("<div name='0' ></div>");
    theElement = doc.RootElement();
    abool = theElement->BoolAttribute("name", 0);
    EXPECT_EQ(abool, 0);
    doc.Parse("<div name='1.5' ></div>");
    theElement = doc.RootElement();
    afloat = theElement->FloatAttribute("name", 0);
    EXPECT_EQ(afloat, 1.5);
    doc.Parse("<div name='1.5' ></div>");
    theElement = doc.RootElement();
    adouble = theElement->DoubleAttribute("name", 0);
    EXPECT_EQ(adouble, 1.5);

    // --------- GetText ----------- //
    const char* tp = NULL;
    doc.Parse("<div><!-- asdfaf -->asdf asdfas adsfasdf</div>");
    theElement = doc.RootElement();
    tp = theElement->GetText();
    EXPECT_STREQ(tp, "asdf asdfas adsfasdf");

    doc.Parse("<div></div>");
    theElement = doc.RootElement();
    tp = theElement->GetText();
    EXPECT_STREQ(tp, NULL);

    doc.Parse("<div><fuck/></div>");
    theElement = doc.RootElement();
    tp = theElement->GetText();
    EXPECT_STREQ(tp, NULL);

    // --------- SetText ----------- //
    doc.Parse("<div></div>");
    theElement = doc.RootElement();
    theElement -> SetText("Hello");
    EXPECT_STREQ(theElement -> GetText(), "Hello");

    doc.Parse("<div>ABCD</div>");
    theElement = doc.RootElement();
    theElement -> SetText("Hello");
    EXPECT_STREQ(theElement -> GetText(), "Hello");

    doc.Parse("<div><ABCD/></div>");
    theElement = doc.RootElement();
    theElement -> SetText("Hello");
    EXPECT_STREQ(theElement -> GetText(), "Hello");

    doc.Parse("<div><!-- adfa --></div>");
    theElement = doc.RootElement();
    theElement -> SetText("Hello");
    EXPECT_STREQ(theElement -> GetText(), "Hello");


    doc.Parse("<div></div>");
    theElement = doc.RootElement();
    theElement -> SetText((int)10);
    EXPECT_STREQ(theElement -> GetText(), "10");

    doc.Parse("<div></div>");
    theElement = doc.RootElement();
    theElement -> SetText((unsigned)11);
    EXPECT_STREQ(theElement -> GetText(), "11");

    doc.Parse("<div></div>");
    theElement = doc.RootElement();
    theElement -> SetText((int64_t)12);
    EXPECT_STREQ(theElement -> GetText(), "12");

    doc.Parse("<div></div>");
    theElement = doc.RootElement();
    theElement -> SetText((uint64_t)13);
    EXPECT_STREQ(theElement -> GetText(), "13");

    doc.Parse("<div></div>");
    theElement = doc.RootElement();
    theElement -> SetText((bool)0);
    EXPECT_STREQ(theElement -> GetText(), "false");

    doc.Parse("<div></div>");
    theElement = doc.RootElement();
    theElement -> SetText((float)1.5);
    EXPECT_STREQ(theElement -> GetText(), "1.5");

    doc.Parse("<div></div>");
    theElement = doc.RootElement();
    theElement -> SetText((double)2.5);
    EXPECT_STREQ(theElement -> GetText(), "2.5");

    // --------- QueryXXXText ----------- //

    doc.Parse("<div>10</div>");
    theElement = doc.RootElement();
    theElement->QueryIntText(&aint);
    EXPECT_EQ(aint, 10);

    doc.Parse("<div>11</div>");
    theElement = doc.RootElement();
    theElement->QueryUnsignedText(&aunsigned);
    EXPECT_EQ(aunsigned, 11);

    doc.Parse("<div>12</div>");
    theElement = doc.RootElement();
    theElement->QueryInt64Text(&aint64);
    EXPECT_EQ(aint64, 12);

    doc.Parse("<div>13</div>");
    theElement = doc.RootElement();
    theElement->QueryUnsigned64Text(&aunsigned64);
    EXPECT_EQ(aunsigned64, 13);

    doc.Parse("<div>true</div>");
    theElement = doc.RootElement();
    theElement->QueryBoolText(&abool);
    EXPECT_EQ(abool, true);

    doc.Parse("<div>1.5</div>");
    theElement = doc.RootElement();
    theElement->QueryFloatText(&afloat);
    EXPECT_EQ(afloat, 1.5);

    doc.Parse("<div>2.5</div>");
    theElement = doc.RootElement();
    theElement->QueryDoubleText(&adouble);
    EXPECT_EQ(adouble, 2.5);

    EXPECT_EQ(theElement->QueryDoubleText(&adouble), 0);

    doc.Parse("<div>asdfasdf</div>");
    theElement = doc.RootElement();
    theElement->QueryIntText(&aint);
    theElement->QueryInt64Text(&aint64);
    theElement->QueryUnsignedText(&aunsigned);
    theElement->QueryUnsigned64Text(&aunsigned64);
    theElement->QueryBoolText(&abool);
    theElement->QueryFloatText(&afloat);
    theElement->QueryDoubleText(&adouble);
    EXPECT_EQ(theElement->QueryDoubleText(&adouble), 16);

    doc.Parse("<div><!-- adfasd --></div>");
    theElement = doc.RootElement();
    theElement->QueryIntText(&aint);
    theElement->QueryInt64Text(&aint64);
    theElement->QueryUnsignedText(&aunsigned);
    theElement->QueryUnsigned64Text(&aunsigned64);
    theElement->QueryBoolText(&abool);
    theElement->QueryFloatText(&afloat);
    theElement->QueryDoubleText(&adouble);
    EXPECT_EQ(theElement->QueryDoubleText(&adouble), 17);

    doc.Parse("<div></div>");
    theElement = doc.RootElement();
    theElement->QueryIntText(&aint);
    theElement->QueryInt64Text(&aint64);
    theElement->QueryUnsignedText(&aunsigned);
    theElement->QueryUnsigned64Text(&aunsigned64);
    theElement->QueryBoolText(&abool);
    theElement->QueryFloatText(&afloat);
    theElement->QueryDoubleText(&adouble);
    EXPECT_EQ(theElement->QueryDoubleText(&adouble), 17);


    // --------- XXXText ----------- //
    doc.Parse("<div>10</div>");
    theElement = doc.RootElement();
    aint = theElement->IntText(0);
    EXPECT_EQ(aint, 10);
    doc.Parse("<div>11</div>");
    theElement = doc.RootElement();
    aint64 = theElement->Int64Text(0);
    EXPECT_EQ(aint64, 11);
    doc.Parse("<div>12</div>");
    theElement = doc.RootElement();
    aunsigned = theElement->UnsignedText(0);
    EXPECT_EQ(aunsigned, 12);
    doc.Parse("<div>13</div>");
    theElement = doc.RootElement();
    aunsigned64 = theElement->Unsigned64Text(0);
    EXPECT_EQ(aunsigned64, 13);
    doc.Parse("<div>false</div>");
    theElement = doc.RootElement();
    abool = theElement->BoolText(0);
    EXPECT_EQ(abool, 0);
    doc.Parse("<div>1.5</div>");
    theElement = doc.RootElement();
    afloat = theElement->FloatText(0);
    EXPECT_EQ(afloat, 1.5);
    doc.Parse("<div>2.5</div>");
    theElement = doc.RootElement();
    adouble = theElement->DoubleText(0);
    EXPECT_EQ(adouble, 2.5);

    // --------- DeleteAttribute ----------- //
    doc.Parse("<div attrib1='1' attrib2='2' attrib3='3' ></div>");
    theElement = doc.RootElement();
    EXPECT_STREQ(theElement->Attribute("attrib2"), "2");
    theElement->DeleteAttribute( "attrib2" );
    EXPECT_STREQ(theElement->Attribute("attrib2"), NULL);
    EXPECT_STREQ(theElement->Attribute("attrib1"), "1");
    theElement->DeleteAttribute( "attrib1" );
    EXPECT_STREQ(theElement->Attribute("attrib1"), NULL);
    EXPECT_STREQ(theElement->Attribute("attrib3"), "3");
    theElement->DeleteAttribute( "attrib3" );
    EXPECT_STREQ(theElement->Attribute("attrib3"), NULL);
    theElement->DeleteAttribute( "attrib4" );

    // --------- ParseAttributes ----------- //
    doc.Parse("\n<div>\n<div \n");
    doc.Parse("\n<div adf='\0></div>\n");
    doc.Parse("<div attr='red' attr='blue' />");
    doc.Parse("<div attr='red'  /");
    doc.Parse("<div attr='red'  $");


    doc.Parse("<div></div>");
    theElement = doc.RootElement();

    // --------- InsertNewChildElement ----------- //
    theElement->InsertNewChildElement("Hello");

    // --------- InsertNewComment ----------- //
    theElement->InsertNewComment("Hello");

    // --------- InsertNewText ----------- //
    theElement->InsertNewText("Hello");

    // --------- InsertNewDeclaration ----------- //
    theElement->InsertNewDeclaration(NULL);
    theElement->InsertNewDeclaration("Hello");

    // --------- InsertNewUnknown ----------- //
    theElement->InsertNewUnknown("Hello");

    // --------- ShallowClone ----------- //
    XMLNode* theNode = NULL;
    theNode = theElement->ShallowClone(NULL);
    theNode = theElement->ShallowClone(&doc);

    // --------- ShallowEqual ----------- //
    XMLDocument doc2;

    theElement->ShallowEqual(theNode);


    doc.Parse("<!-- adf --><div></div>");
    theElement = doc.RootElement();
    theElement->ShallowEqual(doc.FirstChild());

    doc.Parse("<div></div>");
    doc2.Parse("<div2></div2>");
    theElement = doc.RootElement();
    theElement->ShallowEqual(doc2.FirstChild());

    doc.Parse("<div a1 = 'b'></div>");
    doc2.Parse("<div a1 = 'a'></div>");
    theElement = doc.RootElement();
    theElement->ShallowEqual(doc.FirstChild());
    theElement->ShallowEqual(doc2.FirstChild());

    doc.Parse("<div a1 = 'a'></div>");
    doc2.Parse("<div ></div>");
    theElement = doc.RootElement();
    theElement->ShallowEqual(doc2.FirstChild());

    doc.Parse("<div ></div>");
    doc2.Parse("<div a1 = 'a'></div>");
    theElement = doc.RootElement();
    theElement->ShallowEqual(doc2.FirstChild());

    // --------- Accept ----------- //

}

TEST(TEST_XMLDocument, XMLDocument)
{
    XMLDocument doc, doc2;
    // --------- DeepCopy ----------- //
    doc.Parse("<div><fuck/></div>");
    doc.DeepCopy(&doc2);
    doc.DeepCopy(&doc);

    // --------- DeleteNode ----------- //
    XMLElement* unlinkedRoot = doc.NewElement( "Root" );
    XMLElement* linkedRoot = doc.NewElement( "Root" );
    doc.InsertFirstChild( linkedRoot );
    unlinkedRoot->GetDocument()->DeleteNode( linkedRoot );
    unlinkedRoot->GetDocument()->DeleteNode( unlinkedRoot );

    // --------- LoadFile ----------- //
    doc.LoadFile((const char*)NULL);

    doc.LoadFile("./testxml/filetesat.xml");
    std::FILE* f = std::fopen("./testxml/filetest.xml", "w");
    doc.LoadFile(f);
    std::fclose(f);
    f = std::fopen("./testxml/filetest.xml", "r");
    doc.LoadFile(f);
    std::fclose(f);

    // --------- SaveFile ----------- //
    doc.Parse("<div><fuck/></div>");
    doc.SaveFile((const char*)NULL, false);

    doc.SaveFile("./testxml/onlyread.xml", true);  // ! 这里需要一个只读文件

    doc.SaveFile("./testxml/savefile.xml", true);

    // --------- Parse ----------- //
    doc.Parse("<div><fuck/></div>", 3);
    doc.Parse("\0", 100);
    doc.Parse("    ");
    doc.Parse("<div><fuck/></div>", 0);

    // --------- ShallowEqual ----------- //
    EXPECT_EQ(doc.ShallowEqual(doc.FirstChild()), 0);


    // --------- ErrorStr ----------- //
    doc.Parse("<div><fuck/></div>", 0);
    EXPECT_STREQ(doc.ErrorStr(), "Error=XML_ERROR_EMPTY_DOCUMENT ErrorID=13 (0xd) Line number=0");
    doc.Parse("<div><fuck/></div>");
    EXPECT_STREQ(doc.ErrorStr(), "");

    doc.PrintError();
    EXPECT_STREQ(doc.ErrorName(), "XML_SUCCESS");

    const int id = doc.ErrorID();
    EXPECT_EQ(id, 0);
}



TEST(TEST_XMLPrinter, XMLPrinter)
{
    XMLPrinter printer1;
    XMLDocument doc;
    XMLDocument doc2(false);
    XMLElement* theElement;
    std::FILE* f;
    doc.Parse("<root>"
        "    <child1 foo='bar'/>"
        "    <!-- comment thing -->"
        "    <child2 val='1'>Text</child2>"
        "</root>");

    doc.Print(&printer1);

    const char* passages =
            "<?xml version=\"1.0\" standalone=\"no\" ?>"
            "<passages count=\"006\" formatversion=\"20020620\">"
            "<psg context=\"Line 5 has &quot;quotation &apas; \15  marks&quot; and &apos;apostrophe marks&apos;."
            " It also has &lt;, &gt;, and &amp;, as well as a fake copyright &#xA9;.\"> </psg>"
            "</passages>";

    doc.Parse( passages );
    theElement = doc.RootElement()->FirstChildElement();

    f = fopen("./testxml/writetest.xml", "w");
    XMLPrinter streamer( f );
    bool acceptResult = theElement->Accept( &streamer );
    fclose( f );

    doc2.Parse(passages);
    doc2.Print();

    doc.Parse( "<div attr='' ></div>" );
    doc.Print();

    // --------- PushHeader ----------- //
    printer1.PushHeader(0, 0);
    printer1.PushHeader(1, 0);
    printer1.PushHeader(1, 1);
    printer1.PushHeader(0, 1);
    doc.Print(&printer1);

    // --------- PrepareForNewNode ----------- //
    doc.Parse( "<div>Text</div>" );
    theElement = doc.FirstChildElement();
    XMLElement* newElement = doc.NewElement( "Subelement" );
    theElement->InsertEndChild( newElement );
    doc.Print();


    f = fopen("./testxml/printertest.xml", "w");
    doc.Parse( "<div attr='dark' ><van color='red' />Text</div>" );
    XMLPrinter printer2(f);

    // --------- PushAttribute ----------- //
    printer2.PushAttribute("name", "Hello");
    printer2.PushAttribute("name", (int)10);
    printer2.PushAttribute("name", (unsigned)11);
    printer2.PushAttribute("name", (int64_t)12);
    printer2.PushAttribute("name", (uint64_t)13);
    printer2.PushAttribute("name", (bool)1);
    printer2.PushAttribute("name", (float)1.5);
    printer2.PushAttribute("name", (double)2.5);

    // --------- PushText ----------- //
    printer2.PushText("Hello!", 0);
    printer2.PushText("Hello!", 1);
    printer2.PushText("Hello");
    printer2.PushText((int)10);
    printer2.PushText((unsigned)11);
    printer2.PushText((int64_t)12);
    printer2.PushText((uint64_t)13);
    printer2.PushText((bool)1);
    printer2.PushText((float)1.5);
    printer2.PushText((double)2.5);

    // --------- PushComment ----------- //
    printer2.PushComment("Hello!");

    // --------- void XMLPrinter::PushDeclaration( const char* value ) ----------- //
    printer2.PushDeclaration("Hello!");

    // --------- PushUnknown ----------- //
    printer2.PushUnknown("Hello!");

    // --------- VisitEnter ----------- //

    doc.Parse( "<div attr='dark' ></div>" );

    doc.SetBOM(true);
    printer2.VisitEnter(doc);
    doc.SetBOM(false);
    printer2.VisitEnter(doc);

    printer2.VisitEnter(*doc.NewElement("YJSchaf"), doc.RootElement() -> FirstAttribute());

    doc.Print(&printer2);
    fclose(f);

}

TEST(TEST_XMLVisitor, XMLVisitor)
{
    XMLVisitor theVisitor;
    XMLDocument doc;
    doc.Parse( "<div attr='dark'></div>" );

    EXPECT_EQ(true, theVisitor.VisitEnter(*(const XMLDocument*)&doc));
    EXPECT_EQ(true, theVisitor.VisitExit(*(const XMLDocument*)&doc));
    EXPECT_EQ(true, theVisitor.VisitEnter(*(const XMLElement*)doc.NewElement("div"),
                                           (const XMLAttribute*)doc.RootElement()->FindAttribute("attr")));
    EXPECT_EQ(true, theVisitor.VisitExit(*(const XMLElement*)doc.NewElement("div")));

    EXPECT_EQ(true, theVisitor.Visit(*(const XMLDeclaration*)doc.NewDeclaration("Hello")));
    EXPECT_EQ(true, theVisitor.Visit(*(const XMLText*)doc.NewText("Hello")));
    EXPECT_EQ(true, theVisitor.Visit(*(const XMLComment*)doc.NewComment("Hello")));
    EXPECT_EQ(true, theVisitor.Visit(*(const XMLUnknown*)doc.NewUnknown("Hello")));

}


int main(int argc, char **argv)
{
    srand(time(NULL));
    testing::InitGoogleTest(&argc, argv);
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
