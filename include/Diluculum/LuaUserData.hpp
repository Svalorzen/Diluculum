/******************************************************************************\
* LuaUserData.hpp                                                              *
* A C++ equivalent of a Lua userdata.                                          *
* Leandro Motta Barros                                                         *
\******************************************************************************/

#ifndef _DILUCULUM_LUA_USER_DATA_HPP_
#define _DILUCULUM_LUA_USER_DATA_HPP_

#include <boost/scoped_array.hpp>
#include <lua.hpp>
#include <Diluculum/Types.hpp>


namespace Diluculum
{
   /** A C++ equivalent of a Lua userdata. Or, perhaps more precisely: something
    *  that lives in the C++-space, and that stores data that would normally be
    *  in the Lua-space as userdata. This is used as the value of a \c LuaValue
    *  when it holds userdata.
    *  @note A \c LuaUserData has no counterpart in a Lua state. It is simply a
    *        block of memory that can be (via a \c LuaValue and a \c LuaState)
    *        stored in or read from a userdata in Lua state.
    *  @todo What about metatables?
    */
   class LuaUserData
   {
      public:

         /** Constructs a \c LuaUserData, allocating \c size bytes of memory.
          *  This memory is initially filled with garbage. And this memory is
          *  automatically freed when the \c LuaUserData is destroyed.
          */
         LuaUserData (size_t size);

         /** The copy constructor. The newly constructed \c LuaUserData will
          *  have its own block of memory, with the same contents as the
          *  \c other. In other words, this constructor allocates and copies
          *  memory.
          */
         LuaUserData (const LuaUserData& other);

         /** Assigns a \c LuaUserData to this one. The memory currently
          *  allocated for \c this will be freed, new memory will be allocated,
          *  and the data stored in \c rhs will be copied to \c this.
          */
         const LuaUserData& operator= (const LuaUserData& rhs);

         /** Returns the size, in bytes, of the data stored in this
          *  \c LuaUserData.
          */
         size_t getSize() const { return size_; }

         /// Returns a pointer to the data stored in this \c LuaUserData.
         void* getData() { return data_.get(); }

         /** Returns a \c const pointer to the data stored in this
          *  \c LuaUserData.
          */
         const void* getData() const { return data_.get(); }

         /** The "greater than" operator for \c LuaUserData.
          *  @note Given two <tt>LuaUserData</tt>s, the decision on which one is
          *        greater is somewhat arbitrary. Here, the userdata with larger
          *        \c size() is considered greater. If both are equal, the
          *        decision is based on the contents of the stored data.
          *  @todo This is currently implemented using \c memcmp(). I think that
          *        this is not part of the C++ standard yet (just of C99).
          */
         bool operator> (const LuaUserData& rhs);

         /** The "less than" operator for \c LuaUserData.
          *  @note Given two <tt>LuaUserData</tt>s, the decision on which one is
          *        lesser is somewhat arbitrary. The criterion is similar to the
          *        described for the "greater than" operator.
          *  @todo This is currently implemented using \c memcmp(). I think that
          *        this is not part of the C++ standard yet (just of C99).
          */
         bool operator< (const LuaUserData& rhs);

         /** The "equal to" operator for \c LuaUserData.
          *  @note Two <tt>LuaUserData</tt>s are considered equal if the data
          *        they store have the same size and the same contents.
          *  @todo In Lua, a userdata is considered equal only to itself. Things
          *        are different here. Does this have a reason to not be like in
          *        Lua?
          */
         bool operator== (const LuaUserData& rhs);

         /// The "different than" operator for \c LuaUserData.
         bool operator!= (const LuaUserData& rhs);

      private:
         /// The number of bytes stored "in" \c data_.
         size_t size_;

         /// A (smart) pointer to the data owned by this \c LuaUserData.
         boost::scoped_array<char> data_;
   };

} // namespace Diluculum

#endif // _DILUCULUM_LUA_USER_DATA_HPP_