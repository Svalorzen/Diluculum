/******************************************************************************\
* LuaValue.cpp                                                                 *
* A class that somewhat mimics a Lua value.                                    *
* Leandro Motta Barros                                                         *
\******************************************************************************/

#include "LuaValue.hpp"

namespace Diluculum
{

   // - TypeMismatchError::TypeMismatchError -----------------------------------
   TypeMismatchError::TypeMismatchError (const std::string& expectedType,
                                         const std::string& foundType)
      : LuaValueError (("Type mismatch: '" + expectedType
                        + "' was expected but '" + foundType
                        + "' was found.").c_str()),
        expectedType_ (expectedType), foundType_(foundType)
   { }



   // - NoSuchKeyError::NoSuchKeyError -----------------------------------------
   NoSuchKeyError::NoSuchKeyError (const LuaValue& badKey)
      : LuaValueError ("Trying to access a table with an invalid key."),
        badKey_ (badKey)
   { }



   // - LuaValue::type ---------------------------------------------------------
   int LuaValue::type() const
   {
      if (value_.type() == typeid (NilType))
         return LUA_TNIL;
      else if (value_.type() == typeid (bool))
         return LUA_TBOOLEAN;
      else if (value_.type() == typeid (lua_Number))
         return LUA_TNUMBER;
      else if (value_.type() == typeid (std::string))
         return LUA_TSTRING;
      else if (value_.type() == typeid (LuaValueMap))
         return LUA_TTABLE;
      else
         return LUA_TNONE;
   }



   // - LuaValue::typeName -----------------------------------------------------
   std::string LuaValue::typeName() const
   {
      if (value_.type() == typeid (NilType))
         return "nil";
      else if (value_.type() == typeid (bool))
         return "boolean";
      else if (value_.type() == typeid (lua_Number))
         return "number";
      else if (value_.type() == typeid (std::string))
         return "string";
      else if (value_.type() == typeid (LuaValueMap))
         return "table";
      else
         return "";
   }



   // - LuaValue::asNumber() ---------------------------------------------------
   lua_Number LuaValue::asNumber() const
   {
      try
      {
         return boost::get<lua_Number>(value_);
      }
      catch (boost::bad_get& e)
      {
         throw TypeMismatchError ("number", typeName());
      }
   }



   // - LuaValue::asString -----------------------------------------------------
   std::string LuaValue::asString() const
   {
      try
      {
         return boost::get<std::string>(value_);
      }
      catch (boost::bad_get& e)
      {
         throw TypeMismatchError ("string", typeName());
      }
   }



   // - LuaValue::asBoolean ----------------------------------------------------
   bool LuaValue::asBoolean() const
   {
      try
      {
         return boost::get<bool>(value_);
      }
      catch (boost::bad_get& e)
      {
         throw TypeMismatchError ("boolean", typeName());
      }
   }



   // - LuaValue::asTable ------------------------------------------------------
   LuaValueMap LuaValue::asTable() const
   {
      try
      {
         return boost::get<LuaValueMap>(value_);
      }
      catch (boost::bad_get& e)
      {
         throw TypeMismatchError ("table", typeName());
      }
   }



   // - LuaValue::operator< ----------------------------------------------------
   bool LuaValue::operator< (const LuaValue& rhs) const
   {
      std::string lhsTypeName = typeName();
      std::string rhsTypeName = rhs.typeName();

      if (lhsTypeName < rhsTypeName)
         return true;
      else if (lhsTypeName > rhsTypeName)
         return false;
      else // lhsTypeName == rhsTypeName
      {
         if (lhsTypeName == "nil")
            return false;
         else if (lhsTypeName == "boolean")
            return asBoolean() < rhs.asBoolean();
         else if (lhsTypeName == "number")
            return asNumber() < rhs.asNumber();
         else if (lhsTypeName == "string")
            return asString() < rhs.asString();
         else if (lhsTypeName == "table")
         {
            const LuaValueMap lhsMap = asTable();
            const LuaValueMap rhsMap = rhs.asTable();

            if (lhsMap.size() < rhsMap.size())
               return true;
            else if (lhsMap.size() > rhsMap.size())
               return false;
            else
            {
               typedef LuaValueMap::const_iterator iter_t;

               iter_t pLHS = lhsMap.begin();
               iter_t pRHS = rhsMap.begin();
               const iter_t end = lhsMap.end();

               while (pLHS != end)
               {
                  // check the key first
                  if (pLHS->first < pRHS->first)
                     return true;
                  else if (pLHS->first > pRHS->first)
                     return false;

                  // then check the value
                  if (pLHS->second < pRHS->second)
                     return true;
                  else if (pLHS->second > pRHS->second)
                     return false;

                  // make the iterators iterate
                  ++pRHS;
                  ++pLHS;
               }
               return false;
            }
         }
         else // typename == ""
            return false;
      }
   }



   // - LuaValue::operator> ----------------------------------------------------
   bool LuaValue::operator> (const LuaValue& rhs) const
   {
      std::string lhsTypeName = typeName();
      std::string rhsTypeName = rhs.typeName();

      if (lhsTypeName > rhsTypeName)
         return true;
      else if (lhsTypeName < rhsTypeName)
         return false;
      else // lhsTypeName == rhsTypeName
      {
         if (lhsTypeName == "nil")
            return false;
         else if (lhsTypeName == "boolean")
            return asBoolean() > rhs.asBoolean();
         else if (lhsTypeName == "number")
            return asNumber() > rhs.asNumber();
         else if (lhsTypeName == "string")
            return asString() > rhs.asString();
         else if (lhsTypeName == "table")
         {
            const LuaValueMap lhsMap = asTable();
            const LuaValueMap rhsMap = rhs.asTable();

            if (lhsMap.size() > rhsMap.size())
               return true;
            else if (lhsMap.size() < rhsMap.size())
               return false;
            else
            {
               typedef LuaValueMap::const_iterator iter_t;

               iter_t pLHS = lhsMap.begin();
               iter_t pRHS = rhsMap.begin();
               const iter_t end = lhsMap.end();

               while (pLHS != end)
               {
                  // check the key first
                  if (pLHS->first > pRHS->first)
                     return true;
                  else if (pLHS->first < pRHS->first)
                     return false;

                  // then check the value
                  if (pLHS->second > pRHS->second)
                     return true;
                  else if (pLHS->second < pRHS->second)
                     return false;

                  // make the iterators iterate
                  ++pRHS;
                  ++pLHS;
               }
               return false;
            }
         }
         else // typename == ""
            return false;
      }
   }



   // - LuaValue::LuaValue -----------------------------------------------------
   LuaValue& LuaValue::operator[] (const LuaValue& key)
   {
      if (type() != LUA_TTABLE)
         throw TypeMismatchError ("table", typeName());

      LuaValueMap& table = boost::get<LuaValueMap>(value_);

      return table[key];
   }



   // - LuaValue::LuaValue -----------------------------------------------------
   const LuaValue& LuaValue::operator[] (const LuaValue& key) const
   {
      if (type() != LUA_TTABLE)
         throw TypeMismatchError ("table", typeName());

      const LuaValueMap& table = boost::get<LuaValueMap>(value_);

      LuaValueMap::const_iterator it = table.find(key);

      if (it == table.end())
         throw NoSuchKeyError (key);

      return it->second;
   }

} // namespace Diluculum