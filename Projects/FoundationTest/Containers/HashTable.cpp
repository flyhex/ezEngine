#include <PCH.h>

namespace
{
  typedef ezConstructionCounter st;

  struct Collision
  {
    ezUInt32 hash;
    int key;

    inline Collision(ezUInt32 hash, int key)
    {
      this->hash = hash;
      this->key = key;
    }

    inline bool operator==(const Collision& other) const
    {
      return key == other.key;
    }

    EZ_DECLARE_POD_TYPE();
  };
}

template <>
struct ezHashHelper<Collision>
{
  EZ_FORCE_INLINE static ezUInt32 Hash(const Collision& value)
  {
    return value.hash;
  }
};

template <>
struct ezHashHelper<st>
{
  EZ_FORCE_INLINE static ezUInt32 Hash(const st& value)
  {
    return ezHashHelper<ezInt32>::Hash(value.m_iData);
  }
};

EZ_CREATE_SIMPLE_TEST(Containers, HashTable)
{
  EZ_TEST_BLOCK(true, "Constructor")
  {
    ezHashTable<ezInt32, st> table1;

    EZ_TEST(table1.GetCount() == 0);
    EZ_TEST(table1.IsEmpty());
  }

  EZ_TEST_BLOCK(true, "Copy Constructor/Assignment")
  {
    ezHashTable<ezInt32, st> table1;

    for (ezInt32 i = 0; i < 32; ++i)
    {
      table1.Insert(rand() % 100000, ezConstructionCounter(i));
    }

    ezHashTable<ezInt32, st> table2;
    table2 = table1;
    ezHashTable<ezInt32, st> table3(table1);

    EZ_TEST_INT(table1.GetCount(), 32);
    EZ_TEST_INT(table2.GetCount(), 32);
    EZ_TEST_INT(table3.GetCount(), 32);

    ezInt32 i = 0;
    for (ezHashTable<ezInt32, st>::Iterator it = table1.GetIterator(); it.IsValid(); ++it)
    {
      EZ_TEST(it.Value() == table2[it.Key()]);
      EZ_TEST(it.Value() == table3[it.Key()]);
      ++i;
    }
  }

  EZ_TEST_BLOCK(true, "Collision Tests")
  {
    ezHashTable<Collision, int> map2;

    map2[Collision(0, 0)] = 0;
    map2[Collision(1, 1)] = 1;
    map2[Collision(0, 2)] = 2;
    map2[Collision(1, 3)] = 3;
    map2[Collision(1, 4)] = 4;
    map2[Collision(0, 5)] = 5;

    EZ_TEST(map2[Collision(0, 0)] == 0);
    EZ_TEST(map2[Collision(1, 1)] == 1);
    EZ_TEST(map2[Collision(0, 2)] == 2);
    EZ_TEST(map2[Collision(1, 3)] == 3);
    EZ_TEST(map2[Collision(1, 4)] == 4);
    EZ_TEST(map2[Collision(0, 5)] == 5);

    EZ_TEST(map2.KeyExists(Collision(0, 0)));
    EZ_TEST(map2.KeyExists(Collision(1, 1)));
    EZ_TEST(map2.KeyExists(Collision(0, 2)));
    EZ_TEST(map2.KeyExists(Collision(1, 3)));
    EZ_TEST(map2.KeyExists(Collision(1, 4)));
    EZ_TEST(map2.KeyExists(Collision(0, 5)));

    EZ_TEST(map2.Remove(Collision(0, 0)));
    EZ_TEST(map2.Remove(Collision(1, 1)));

    EZ_TEST(map2[Collision(0, 2)] == 2);
    EZ_TEST(map2[Collision(1, 3)] == 3);
    EZ_TEST(map2[Collision(1, 4)] == 4);
    EZ_TEST(map2[Collision(0, 5)] == 5);

    EZ_TEST(!map2.KeyExists(Collision(0, 0)));
    EZ_TEST(!map2.KeyExists(Collision(1, 1)));
    EZ_TEST(map2.KeyExists(Collision(0, 2)));
    EZ_TEST(map2.KeyExists(Collision(1, 3)));
    EZ_TEST(map2.KeyExists(Collision(1, 4)));
    EZ_TEST(map2.KeyExists(Collision(0, 5)));

    map2[Collision(0, 6)] = 6;
    map2[Collision(1, 7)] = 7;

    EZ_TEST(map2[Collision(0, 2)] == 2);
    EZ_TEST(map2[Collision(1, 3)] == 3);
    EZ_TEST(map2[Collision(1, 4)] == 4);
    EZ_TEST(map2[Collision(0, 5)] == 5);
    EZ_TEST(map2[Collision(0, 6)] == 6);
    EZ_TEST(map2[Collision(1, 7)] == 7);

    EZ_TEST(map2.KeyExists(Collision(0, 2)));
    EZ_TEST(map2.KeyExists(Collision(1, 3)));
    EZ_TEST(map2.KeyExists(Collision(1, 4)));
    EZ_TEST(map2.KeyExists(Collision(0, 5)));
    EZ_TEST(map2.KeyExists(Collision(0, 6)));
    EZ_TEST(map2.KeyExists(Collision(1, 7)));

    EZ_TEST(map2.Remove(Collision(1, 4)));
    EZ_TEST(map2.Remove(Collision(0, 6)));

    EZ_TEST(map2[Collision(0, 2)] == 2);
    EZ_TEST(map2[Collision(1, 3)] == 3);
    EZ_TEST(map2[Collision(0, 5)] == 5);
    EZ_TEST(map2[Collision(1, 7)] == 7);

    EZ_TEST(!map2.KeyExists(Collision(1, 4)));
    EZ_TEST(!map2.KeyExists(Collision(0, 6)));
    EZ_TEST(map2.KeyExists(Collision(0, 2)));
    EZ_TEST(map2.KeyExists(Collision(1, 3)));
    EZ_TEST(map2.KeyExists(Collision(0, 5)));
    EZ_TEST(map2.KeyExists(Collision(1, 7)));

    map2[Collision(0, 2)] = 3;
    map2[Collision(0, 5)] = 6;
    map2[Collision(1, 3)] = 4;

    EZ_TEST(map2[Collision(0, 2)] == 3);
    EZ_TEST(map2[Collision(0, 5)] == 6);
    EZ_TEST(map2[Collision(1, 3)] == 4);
  }

  EZ_TEST_BLOCK(true, "Clear")
  {
    EZ_TEST(st::HasAllDestructed());

    {
      ezHashTable<ezUInt32, st> m1;
      m1[0] = st(1);
      EZ_TEST(st::HasDone(2, 1)); // for inserting new elements 1 temporary is created (and destroyed)

      m1[1] = st(3);
      EZ_TEST(st::HasDone(2, 1)); // for inserting new elements 2 temporary is created (and destroyed)

      m1[0] = st(2);
      EZ_TEST(st::HasDone(1, 1)); // nothing new to create, so only the one temporary is used

      m1.Clear();
      EZ_TEST(st::HasDone(0, 2));
      EZ_TEST(st::HasAllDestructed());
    }

    {
      ezHashTable<st, ezUInt32> m1;
      m1[st(0)] = 1;
      EZ_TEST(st::HasDone(2, 1)); // one temporary

      m1[st(1)] = 3;
      EZ_TEST(st::HasDone(2, 1)); // one temporary

      m1[st(0)] = 2;
      EZ_TEST(st::HasDone(1, 1)); // nothing new to create, so only the one temporary is used

      m1.Clear();
      EZ_TEST(st::HasDone(0, 2));
      EZ_TEST(st::HasAllDestructed());
    }
  }

  EZ_TEST_BLOCK(true, "Insert/TryGetValue")
  {
    ezHashTable<ezInt32, st> a1;

    for (ezInt32 i = 0; i < 10; ++i)
    {
      EZ_TEST(!a1.Insert(i, i - 20));
    }

    for (ezInt32 i = 0; i < 10; ++i)
    {
      st oldValue;
      EZ_TEST(a1.Insert(i, i, &oldValue));
      EZ_TEST_INT(oldValue.m_iData, i - 20);
    }

    st value;
    EZ_TEST(a1.TryGetValue(9, value));
    EZ_TEST_INT(value.m_iData, 9);

    EZ_TEST(!a1.TryGetValue(11, value));
    EZ_TEST_INT(value.m_iData, 9);

    st* pValue;
    EZ_TEST(a1.TryGetValue(9, pValue));
    EZ_TEST_INT(pValue->m_iData, 9);

    pValue->m_iData = 20;
    EZ_TEST_INT(a1[9].m_iData, 20);
  }

  EZ_TEST_BLOCK(true, "Remove/Compact")
  {
    ezHashTable<ezInt32, st> a;

    for (ezInt32 i = 0; i < 1000; ++i)
    {
      a.Insert(i, i);
      EZ_TEST_INT(a.GetCount(), i + 1);
    }

    a.Compact();

    for (ezInt32 i = 0; i < 1000; ++i)
      EZ_TEST_INT(a[i].m_iData, i);


    for (ezInt32 i = 0; i < 500; ++i)
    {
      st oldValue;
      EZ_TEST(a.Remove(i, &oldValue));
      EZ_TEST_INT(oldValue.m_iData, i);
    }
    
    a.Compact();

    for (ezInt32 i = 500; i < 1000; ++i)
      EZ_TEST_INT(a[i].m_iData, i);
  }
}
