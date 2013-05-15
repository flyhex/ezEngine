#include <PCH.h>

struct ezConstructTest
{
public:
  ezConstructTest()
  {
    m_iData = 42;
  }

  ~ezConstructTest()
  {
    m_iData = 23;
  }

  ezInt32 m_iData;
};

static const ezUInt32 uiSize = sizeof(ezConstructTest);

EZ_CREATE_SIMPLE_TEST(Memory, MemoryUtils)
{
  EZ_TEST_BLOCK(true, "Construct")
  {
    ezUInt8 uiRawData[uiSize * 5] = { 0 };
    ezConstructTest* pTest = (ezConstructTest*)(uiRawData);

    ezMemoryUtils::Construct<ezConstructTest>(pTest + 1, 2);

    EZ_TEST_INT(pTest[0].m_iData, 0);
    EZ_TEST_INT(pTest[1].m_iData, 42);
    EZ_TEST_INT(pTest[2].m_iData, 42);
    EZ_TEST_INT(pTest[3].m_iData, 0);
    EZ_TEST_INT(pTest[4].m_iData, 0);
  }

  EZ_TEST_BLOCK(true, "Construct Copy(Array)")
  {
    ezUInt8 uiRawData[uiSize * 5] = { 0 };
    ezConstructTest* pTest = (ezConstructTest*)(uiRawData);

    ezConstructTest Copy[2];
    Copy[0].m_iData = 43;
    Copy[1].m_iData = 44;

    ezMemoryUtils::Construct<ezConstructTest>(pTest + 1, Copy, 2);

    EZ_TEST_INT(pTest[0].m_iData, 0);
    EZ_TEST_INT(pTest[1].m_iData, 43);
    EZ_TEST_INT(pTest[2].m_iData, 44);
    EZ_TEST_INT(pTest[3].m_iData, 0);
    EZ_TEST_INT(pTest[4].m_iData, 0);
  }

  EZ_TEST_BLOCK(true, "Construct Copy(Element)")
  {
    ezUInt8 uiRawData[uiSize * 5] = { 0 };
    ezConstructTest* pTest = (ezConstructTest*)(uiRawData);

    ezConstructTest Copy;
    Copy.m_iData = 43;

    ezMemoryUtils::Construct<ezConstructTest>(pTest + 1, Copy, 2);

    EZ_TEST_INT(pTest[0].m_iData, 0);
    EZ_TEST_INT(pTest[1].m_iData, 43);
    EZ_TEST_INT(pTest[2].m_iData, 43);
    EZ_TEST_INT(pTest[3].m_iData, 0);
    EZ_TEST_INT(pTest[4].m_iData, 0);
  }

  EZ_TEST_BLOCK(true, "Destruct")
  {
    ezUInt8 uiRawData[uiSize * 5] = { 0 };
    ezConstructTest* pTest = (ezConstructTest*)(uiRawData);

    ezMemoryUtils::Construct<ezConstructTest>(pTest + 1, 2);

    EZ_TEST_INT(pTest[0].m_iData, 0);
    EZ_TEST_INT(pTest[1].m_iData, 42);
    EZ_TEST_INT(pTest[2].m_iData, 42);
    EZ_TEST_INT(pTest[3].m_iData, 0);
    EZ_TEST_INT(pTest[4].m_iData, 0);

    ezMemoryUtils::Destruct<ezConstructTest>(pTest, 4);

    EZ_TEST_INT(pTest[0].m_iData, 23);
    EZ_TEST_INT(pTest[1].m_iData, 23);
    EZ_TEST_INT(pTest[2].m_iData, 23);
    EZ_TEST_INT(pTest[3].m_iData, 23);
    EZ_TEST_INT(pTest[4].m_iData, 0);
  }

  EZ_TEST_BLOCK(true, "Copy")
  {
    ezUInt8 uiRawData[5]  = { 1, 2, 3, 4, 5 };
    ezUInt8 uiRawData2[5] = { 6, 7, 8, 9, 0 };
    
    EZ_TEST_INT(uiRawData[0], 1);
    EZ_TEST_INT(uiRawData[1], 2);
    EZ_TEST_INT(uiRawData[2], 3);
    EZ_TEST_INT(uiRawData[3], 4);
    EZ_TEST_INT(uiRawData[4], 5);

    ezMemoryUtils::Copy(uiRawData + 1, uiRawData2 + 2, 3);

    EZ_TEST_INT(uiRawData[0], 1);
    EZ_TEST_INT(uiRawData[1], 8);
    EZ_TEST_INT(uiRawData[2], 9);
    EZ_TEST_INT(uiRawData[3], 0);
    EZ_TEST_INT(uiRawData[4], 5);
  }

  EZ_TEST_BLOCK(true, "Move")
  {
    ezUInt8 uiRawData[5]  = { 1, 2, 3, 4, 5 };
    
    EZ_TEST_INT(uiRawData[0], 1);
    EZ_TEST_INT(uiRawData[1], 2);
    EZ_TEST_INT(uiRawData[2], 3);
    EZ_TEST_INT(uiRawData[3], 4);
    EZ_TEST_INT(uiRawData[4], 5);

    ezMemoryUtils::Move(uiRawData + 1, uiRawData + 3, 2);

    EZ_TEST_INT(uiRawData[0], 1);
    EZ_TEST_INT(uiRawData[1], 4);
    EZ_TEST_INT(uiRawData[2], 5);
    EZ_TEST_INT(uiRawData[3], 4);
    EZ_TEST_INT(uiRawData[4], 5);

    ezMemoryUtils::Move(uiRawData + 1, uiRawData, 4);

    EZ_TEST_INT(uiRawData[0], 1);
    EZ_TEST_INT(uiRawData[1], 1);
    EZ_TEST_INT(uiRawData[2], 4);
    EZ_TEST_INT(uiRawData[3], 5);
    EZ_TEST_INT(uiRawData[4], 4);
  }

  EZ_TEST_BLOCK(true, "ZeroFill")
  {
    ezUInt8 uiRawData[5] = { 1, 2, 3, 4, 5 };
    
    EZ_TEST_INT(uiRawData[0], 1);
    EZ_TEST_INT(uiRawData[1], 2);
    EZ_TEST_INT(uiRawData[2], 3);
    EZ_TEST_INT(uiRawData[3], 4);
    EZ_TEST_INT(uiRawData[4], 5);

    ezMemoryUtils::ZeroFill(uiRawData + 1, 3);

    EZ_TEST_INT(uiRawData[0], 1);
    EZ_TEST_INT(uiRawData[1], 0);
    EZ_TEST_INT(uiRawData[2], 0);
    EZ_TEST_INT(uiRawData[3], 0);
    EZ_TEST_INT(uiRawData[4], 5);
  }

  EZ_TEST_BLOCK(true, "IsEqual")
  {
    ezUInt8 uiRawData1[5] = { 1, 2, 3, 4, 5 };
    ezUInt8 uiRawData2[5] = { 1, 2, 3, 4, 5 };
    ezUInt8 uiRawData3[5] = { 1, 2, 3, 4, 6 };

    EZ_TEST(ezMemoryUtils::IsEqual(uiRawData1, uiRawData2, 5));
    EZ_TEST(!ezMemoryUtils::IsEqual(uiRawData1, uiRawData3, 5));
    EZ_TEST(ezMemoryUtils::IsEqual(uiRawData1, uiRawData3, 4));
  }

  EZ_TEST_BLOCK(true, "AddByteOffset")
  {
    ezInt32* pData1 = NULL;
    pData1 = ezMemoryUtils::AddByteOffset(pData1, 13);
    EZ_TEST_INT(pData1, 13);

    const ezInt32* pData2 = NULL;
    const ezInt32* pData3 = ezMemoryUtils::AddByteOffsetConst(pData2, 17);
    EZ_TEST(pData3 == reinterpret_cast<ezInt32*>(17));
  }

  EZ_TEST_BLOCK(true, "Align / IsAligned")
  {
    {
      ezInt32* pData = (ezInt32*) 1;
      EZ_TEST(!ezMemoryUtils::IsAligned(pData, 4));
      pData = ezMemoryUtils::Align(pData, 4);
      EZ_TEST(pData == reinterpret_cast<ezInt32*>(0));
      EZ_TEST(ezMemoryUtils::IsAligned(pData, 4));
    }
    {
      ezInt32* pData = (ezInt32*) 2;
      EZ_TEST(!ezMemoryUtils::IsAligned(pData, 4));
      pData = ezMemoryUtils::Align(pData, 4);
      EZ_TEST(pData == reinterpret_cast<ezInt32*>(0));
      EZ_TEST(ezMemoryUtils::IsAligned(pData, 4));
    }
    {
      ezInt32* pData = (ezInt32*) 3;
      EZ_TEST(!ezMemoryUtils::IsAligned(pData, 4));
      pData = ezMemoryUtils::Align(pData, 4);
      EZ_TEST(pData == reinterpret_cast<ezInt32*>(0));
      EZ_TEST(ezMemoryUtils::IsAligned(pData, 4));
    }
    {
      ezInt32* pData = (ezInt32*) 4;
      EZ_TEST(ezMemoryUtils::IsAligned(pData, 4));
      pData = ezMemoryUtils::Align(pData, 4);
      EZ_TEST(pData == reinterpret_cast<ezInt32*>(4));
      EZ_TEST(ezMemoryUtils::IsAligned(pData, 4));
    }
  }
}

