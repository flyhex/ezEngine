﻿#include <PCH.h>

EZ_CREATE_SIMPLE_TEST(Strings, UnicodeUtils)
{
  EZ_TEST_BLOCK(true, "IsASCII")
  {
    // test all ASCII Characters
    for (ezUInt32 i = 0; i < 128; ++i)
      EZ_TEST(ezUnicodeUtils::IsASCII(i));

    for (ezUInt32 i = 128; i < 0xFFFFF; ++i)
      EZ_TEST(!ezUnicodeUtils::IsASCII(i));
  }

  EZ_TEST_BLOCK(true, "IsUtf8ContinuationByte")
  {
    // all ASCII Characters are not continuation bytes
    for (ezUInt32 i = 0; i < 128; ++i)
      EZ_TEST(!ezUnicodeUtils::IsUtf8ContinuationByte(i));

    for (ezUInt32 i = 0; i < 255; ++i)
    {
      const ezUInt32 uiContByte = 0x80 | (i & 0x3f);
      const ezUInt32 uiNoContByte1 = i | 0x40;
      const ezUInt32 uiNoContByte2 = i | 0xC0;

      EZ_TEST(ezUnicodeUtils::IsUtf8ContinuationByte(uiContByte));
      EZ_TEST(!ezUnicodeUtils::IsUtf8ContinuationByte(uiNoContByte1));
      EZ_TEST(!ezUnicodeUtils::IsUtf8ContinuationByte(uiNoContByte2));
    }
  }

  EZ_TEST_BLOCK(true, "GetUtf8SequenceLength")
  {
    // All ASCII characters are 1 byte in length
    for (ezUInt32 i = 0; i < 128; ++i)
      EZ_TEST_INT(ezUnicodeUtils::GetUtf8SequenceLength(i), 1);

    // Todo: ... auch UTF-8 Zeichen testen...
  }

  EZ_TEST_BLOCK(true, "ConvertUtf8ToUtf32")
  {
    // Just wraps around 'utf8::peek_next'
    // I think we can assume that that works.
  }

  EZ_TEST_BLOCK(true, "GetSizeForCharacterInUtf8")
  {
    // All ASCII characters are 1 byte in length
    for (ezUInt32 i = 0; i < 128; ++i)
      EZ_TEST_INT(ezUnicodeUtils::GetSizeForCharacterInUtf8(i), 1);

    // Todo: Could test this more throughly, although that function is really trivial.
  }

  EZ_TEST_BLOCK(true, "Decode")
  {
    char utf8[] = { 'a', 0 };
    ezUInt16 utf16[] = { 'a', 0 };
    wchar_t wchar[] = { L'a', 0 };

    char* szUtf8 = &utf8[0];
    ezUInt16* szUtf16 = &utf16[0];
    wchar_t* szWChar = &wchar[0];
    
    ezUInt32 uiUtf321 = ezUnicodeUtils::DecodeUtf8ToUtf32(szUtf8);
    ezUInt32 uiUtf322 = ezUnicodeUtils::DecodeUtf16ToUtf32(szUtf16);
    ezUInt32 uiUtf323 = ezUnicodeUtils::DecodeWCharToUtf32(szWChar);

    EZ_TEST_INT(uiUtf321, uiUtf322);
    EZ_TEST_INT(uiUtf321, uiUtf323);
  }

  EZ_TEST_BLOCK(true, "Encode")
  {
    char utf8[4] = { 0 };
    ezUInt16 utf16[4] = { 0 };
    wchar_t wchar[4] = { 0 };

    char* szUtf8 = &utf8[0];
    ezUInt16* szUtf16 = &utf16[0];
    wchar_t* szWChar = &wchar[0];

    ezUnicodeUtils::EncodeUtf32ToUtf8('a', szUtf8);
    ezUnicodeUtils::EncodeUtf32ToUtf16('a', szUtf16);
    ezUnicodeUtils::EncodeUtf32ToWChar('a', szWChar);

    EZ_TEST(utf8[0] == 'a');
    EZ_TEST(utf16[0] == 'a');
    EZ_TEST(wchar[0] == 'a');
  }

  EZ_TEST_BLOCK(true, "MoveToNextUtf8")
  {
    ezStringUtf8 s(L"aböäß€de");

    const char* sz = s.GetData();

    // test how far it skips ahead

    ezUnicodeUtils::MoveToNextUtf8(sz);
    EZ_TEST(sz == &s.GetData()[1]);

    ezUnicodeUtils::MoveToNextUtf8(sz);
    EZ_TEST(sz == &s.GetData()[2]);

    ezUnicodeUtils::MoveToNextUtf8(sz);
    EZ_TEST(sz == &s.GetData()[4]);

    ezUnicodeUtils::MoveToNextUtf8(sz);
    EZ_TEST(sz == &s.GetData()[6]);

    ezUnicodeUtils::MoveToNextUtf8(sz);
    EZ_TEST(sz == &s.GetData()[8]);

    ezUnicodeUtils::MoveToNextUtf8(sz);
    EZ_TEST(sz == &s.GetData()[11]);

    ezUnicodeUtils::MoveToNextUtf8(sz);
    EZ_TEST(sz == &s.GetData()[12]);
  }

  EZ_TEST_BLOCK(true, "MoveToPriorUtf8")
  {
    ezStringUtf8 s(L"aböäß€de");

    const char* sz = &s.GetData()[13];

    // test how far it skips ahead

    ezUnicodeUtils::MoveToPriorUtf8(sz);
    EZ_TEST(sz == &s.GetData()[12]);

    ezUnicodeUtils::MoveToPriorUtf8(sz);
    EZ_TEST(sz == &s.GetData()[11]);

    ezUnicodeUtils::MoveToPriorUtf8(sz);
    EZ_TEST(sz == &s.GetData()[8]);

    ezUnicodeUtils::MoveToPriorUtf8(sz);
    EZ_TEST(sz == &s.GetData()[6]);

    ezUnicodeUtils::MoveToPriorUtf8(sz);
    EZ_TEST(sz == &s.GetData()[4]);

    ezUnicodeUtils::MoveToPriorUtf8(sz);
    EZ_TEST(sz == &s.GetData()[2]);

    ezUnicodeUtils::MoveToPriorUtf8(sz);
    EZ_TEST(sz == &s.GetData()[1]);

    ezUnicodeUtils::MoveToPriorUtf8(sz);
    EZ_TEST(sz == &s.GetData()[0]);
  }

  EZ_TEST_BLOCK(true, "SkipUtf8Bom")
  {
    // C++ is really stupid, chars are signed, but Utf8 only works with unsigned values ... argh!

    char szWithBom[] = { (char) 0xef, (char) 0xbb, (char) 0xbf, 'a' };
    char szNoBom[] = { 'a' };
    const char* pString = szWithBom;

    EZ_TEST(ezUnicodeUtils::SkipUtf8Bom(pString) == true);
    EZ_TEST(pString == &szWithBom[3]);

    pString = szNoBom;

    EZ_TEST(ezUnicodeUtils::SkipUtf8Bom(pString) == false);
    EZ_TEST(pString == szNoBom);
  }

  EZ_TEST_BLOCK(true, "SkipUtf16BomLE")
  {
    ezUInt16 szWithBom[] = { 0xfffe, 'a' };
    ezUInt16 szNoBom[] = { 'a' };

    const ezUInt16* pString = szWithBom;

    EZ_TEST(ezUnicodeUtils::SkipUtf16BomLE(pString) == true);
    EZ_TEST(pString == &szWithBom[1]);

    pString = szNoBom;

    EZ_TEST(ezUnicodeUtils::SkipUtf16BomLE(pString) == false);
    EZ_TEST(pString == szNoBom);
  }

  EZ_TEST_BLOCK(true, "SkipUtf16BomBE")
  {
    ezUInt16 szWithBom[] = { 0xfeff, 'a' };
    ezUInt16 szNoBom[] = { 'a' };

    const ezUInt16* pString = szWithBom;

    EZ_TEST(ezUnicodeUtils::SkipUtf16BomBE(pString) == true);
    EZ_TEST(pString == &szWithBom[1]);

    pString = szNoBom;

    EZ_TEST(ezUnicodeUtils::SkipUtf16BomBE(pString) == false);
    EZ_TEST(pString == szNoBom);
  }
}

