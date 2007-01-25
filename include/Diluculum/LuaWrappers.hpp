/******************************************************************************\
* LuaWrappers.hpp                                                              *
* Making C++ stuff accessible from Lua.                                        *
*                                                                              *
*                                                                              *
* Copyright (C) 2005-2006 by Leandro Motta Barros.                             *
*                                                                              *
* Permission is hereby granted, free of charge, to any person obtaining a copy *
* of this software and associated documentation files (the "Software"), to     *
* deal in the Software without restriction, including without limitation the   *
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or  *
* sell copies of the Software, and to permit persons to whom the Software is   *
* furnished to do so, subject to the following conditions:                     *
*                                                                              *
* The above copyright notice and this permission notice shall be included in   *
* all copies or substantial portions of the Software.                          *
*                                                                              *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR   *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,     *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER       *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING      *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS *
* IN THE SOFTWARE.                                                             *
\******************************************************************************/

#ifndef _DILUCULUM_LUA_WRAPPERS_HPP_
#define _DILUCULUM_LUA_WRAPPERS_HPP_

#include <algorithm>
#include <string>
#include <boost/bind.hpp>
#include <Diluculum/LuaExceptions.hpp>
#include <Diluculum/LuaState.hpp>
#include <Diluculum/LuaUtils.hpp>


namespace Diluculum
{
   namespace Impl
   {
      /** Calls \c lua_error() with a proper error message. The error message is
       *  composed of things that may help to find out what's the error, like
       *  the function name.
       *  @param ls The <tt>lua_State*</tt> on which the function will operate.
       *  @param what A message to be included in the error message created by
       *         this function.
       *  @note This is not intended to be called by Diluculum users.
       */
      void ReportErrorFromCFunction (lua_State* ls, const::std::string& what);



      /** The data that is stored as userdata when a C++ object is exported to
       *  or instantiated in Lua.
       */
      struct CppObject
      {
         public:
            /// Pointer to the actual object.
            void* ptr;

            /** Should the \c ptr be <tt>delete</tt>d when the userdata is
             *  garbage-collected in Lua? Essentially, if the object is
             *  instantiated in Lua, it should be; if it is instantiated in C++,
             *  it doesn't.
             */
            bool deleteMe;
      };



      /** @todo Document this. The idea is that it helps to fill the table
       *        (\c LuaValueMap) representing a class.
       */
      class ClassTableFiller
      {
         public:
            ClassTableFiller (Diluculum::LuaValueMap& classTable,
                              const std::string& name,
                              lua_CFunction func)
            {
               classTable[name] = func;
            }
      };
   }
}



/** Returns the name of the wrapper function that is created by
 *  \c DILUCULUM_WRAP_FUNCTION() for a given function name.
 *  @param FUNC The function whose wrapper name is desired.
 */
#define DILUCULUM_WRAPPER_FUNCTION(FUNC) \
Diluculum__ ## FUNC ## __Wrapper_Function



/** Creates a \c lua_CFunction that wraps a function with the signature like the
 *  following one:
 *  <p><tt>Diluculum::LuaValueList Func (const Diluculum::LuaValueList& params)</tt>
 *  <p>Notice that, thanks to the use of <tt>Diluculum::LuaValueList</tt>s, the
 *  wrapped function can effectively take and return an arbitrary number of
 *  values.
 *  @note The name of the created wrapper function is a decorated version of the
 *        \c FUNC parameter. The decoration scheme can be quite complicated and
 *        is subject to change in future releases of Diluculum, so don't try to
 *        use it directly. Use the \c DILUCULUM_WRAPPER_FUNCTION() macro to
 *        obtain it instead.
 *  @note The proper way to report errors from the function being wrapped is by
 *        <tt>throw</tt>ing a \c Diluculum::LuaError. The created wrapper
 *        function will handle these exceptions and "translate" them to a call
 *        to \c lua_error().
 *  @see DILUCULUM_WRAPPER_FUNCTION() To find out the name of the created
 *       wrapper function.
 *  @param FUNC The function to be wrapped.
 */
#define DILUCULUM_WRAP_FUNCTION(FUNC)                                         \
int DILUCULUM_WRAPPER_FUNCTION(FUNC) (lua_State* ls)                          \
{                                                                             \
   using std::for_each;                                                       \
   using boost::bind;                                                         \
   using Diluculum::PushLuaValue;                                             \
   using Diluculum::Impl::ReportErrorFromCFunction;                           \
                                                                              \
   try                                                                        \
   {                                                                          \
      /* Read parameters and empty the stack */                               \
      const int numParams = lua_gettop (ls);                                  \
      Diluculum::LuaValueList params;                                         \
      for (int i = 1; i <= numParams; ++i)                                    \
         params.push_back (Diluculum::ToLuaValue (ls, i));                    \
      lua_pop (ls, numParams);                                                \
                                                                              \
      /* Call the wrapped function */                                         \
      Diluculum::LuaValueList ret = FUNC (params);                            \
                                                                              \
      /* Push the return values and return */                                 \
      for_each (ret.begin(), ret.end(), bind (PushLuaValue, ls, _1));         \
                                                                              \
      return ret.size();                                                      \
   }                                                                          \
   catch (Diluculum::LuaError& e)                                             \
   {                                                                          \
      ReportErrorFromCFunction (ls, e.what());                                \
      return 0;                                                               \
   }                                                                          \
   catch(...)                                                                 \
   {                                                                          \
      ReportErrorFromCFunction (ls, "Unknown exception caught by wrapper.");  \
      return 0;                                                               \
   }                                                                          \
}



/** Returns the name of the function used to wrap a method \c METHOD of the
 *  class \c CLASS.
 *  @note This is used internally. Users can ignore this macro.
 */
#define DILUCULUM_METHOD_WRAPPER(CLASS, METHOD)                      \
   Diluculum__ ## CLASS ## __ ## METHOD ## __Method_Wrapper_Function



/** Creates a \c lua_CFunction that wraps a method with the signature like the
 *  following one:
 *  <p><tt>Diluculum::LuaValueList Class::Method(
 *  const Diluculum::LuaValueList& params)</tt>
 *  <p>Notice that, thanks to the use of <tt>Diluculum::LuaValueList</tt>s, the
 *  wrapped method can effectively take and return an arbitrary number of
 *  values.
 *  @note The name of the wrapper function is created from the \c CLASS and
 *        \c METHOD parameters. The rule used to generate this name can be quite
 *        complicated and is subject to change in future releases of Diluculum,
 *        so don't try to use it directly. Actually, you don't need it.
 *  @note The proper way to report errors from the method being wrapped is by
 *        <tt>throw</tt>ing a \c Diluculum::LuaError. The created wrapper
 *        function will handle these exceptions and "translate" them to a call
 *        to \c lua_error().
 *  @bug This is no longer to be called by users. So, include it inline where
 *       it is called. Or, at least, rename it, because there may be people
 *       calling this, and the errors will be cryptic.
 *  @param CLASS The class with a method being wrapped.
 *  @param METHOD The method being wrapped.
 */
#define DILUCULUM_WRAP_METHOD(CLASS, METHOD)                                  \
int DILUCULUM_METHOD_WRAPPER(CLASS, METHOD) (lua_State* ls)                   \
{                                                                             \
   using std::for_each;                                                       \
   using boost::bind;                                                         \
   using Diluculum::PushLuaValue;                                             \
   using Diluculum::Impl::CppObject;                                          \
   using Diluculum::Impl::ReportErrorFromCFunction;                           \
                                                                              \
   try                                                                        \
   {                                                                          \
      /* Read parameters and empty the stack */                               \
      const int numParams = lua_gettop (ls);                                  \
      Diluculum::LuaValue ud = Diluculum::ToLuaValue (ls, 1);                 \
      Diluculum::LuaValueList params;                                         \
      for (int i = 2; i <= numParams; ++i)                                    \
         params.push_back (Diluculum::ToLuaValue (ls, i));                    \
      lua_pop (ls, numParams);                                                \
                                                                              \
      /* Get the object pointer and call the method */                        \
      CppObject* cppObj =                                                     \
         reinterpret_cast<CppObject*>(ud.asUserData().getData());             \
      CLASS* pObj = reinterpret_cast<CLASS*>(cppObj->ptr);                    \
                                                                              \
      Diluculum::LuaValueList ret = pObj->METHOD (params);                    \
                                                                              \
      /* Push the return values and return */                                 \
      for_each (ret.begin(), ret.end(), bind (PushLuaValue, ls, _1));         \
                                                                              \
      return ret.size();                                                      \
   }                                                                          \
   catch (Diluculum::LuaError& e)                                             \
   {                                                                          \
      ReportErrorFromCFunction (ls, e.what());                                \
      return 0;                                                               \
   }                                                                          \
   catch(...)                                                                 \
   {                                                                          \
      ReportErrorFromCFunction (ls, "Unknown exception caught by wrapper.");  \
      return 0;                                                               \
   }                                                                          \
}



/** Returns the name of the table that represent the class \c CLASS.
 *  @note This is used internally. Users can ignore this macro.
 */
#define DILUCULUM_CLASS_TABLE(CLASS) \
Diluculum__Class_Table__ ## CLASS



/** Starts a block of class wrapping macro calls. This must be followed by calls
 *  to \c DILUCULUM_CLASS_METHOD() for each method to be exported to Lua and a
 *  final call to \c DILUCULUM_END_CLASS().
 *  @param CLASS The class being exported.
 */
#define DILUCULUM_BEGIN_CLASS(CLASS)                                          \
namespace                                                                     \
{                                                                             \
   /* the table representing the class */                                     \
   Diluculum::LuaValueMap DILUCULUM_CLASS_TABLE(CLASS);                       \
}                                                                             \
                                                                              \
/* The Constructor */                                                         \
int Diluculum__ ## CLASS ## __Constructor_Wrapper_Function (lua_State* ls)    \
{                                                                             \
   using Diluculum::PushLuaValue;                                             \
   using Diluculum::Impl::CppObject;                                          \
   using Diluculum::Impl::ReportErrorFromCFunction;                           \
                                                                              \
   try                                                                        \
   {                                                                          \
      /* Read parameters and empty the stack */                               \
      const int numParams = lua_gettop (ls);                                  \
      Diluculum::LuaValueList params;                                         \
      for (int i = 1; i <= numParams; ++i)                                    \
         params.push_back (Diluculum::ToLuaValue (ls, i));                    \
      lua_pop (ls, numParams);                                                \
                                                                              \
      /* Construct the object, wrap it in a userdata, and return */           \
      void* ud = lua_newuserdata (ls, sizeof(CppObject));                     \
      CppObject* cppObj = reinterpret_cast<CppObject*>(ud);                   \
      cppObj->ptr = new CLASS (params);                                       \
      cppObj->deleteMe = true;                                                \
                                                                              \
      lua_getglobal (ls, #CLASS);                                             \
      lua_setmetatable (ls, -2);                                              \
                                                                              \
      return 1;                                                               \
   }                                                                          \
   catch (Diluculum::LuaError& e)                                             \
   {                                                                          \
      ReportErrorFromCFunction (ls, e.what());                                \
      return 0;                                                               \
   }                                                                          \
   catch(...)                                                                 \
   {                                                                          \
      ReportErrorFromCFunction (ls, "Unknown exception caught by wrapper.");  \
      return 0;                                                               \
   }                                                                          \
}                                                                             \
                                                                              \
/* Destructor */                                                              \
int Diluculum__ ## CLASS ## __Destructor_Wrapper_Function (lua_State* ls)     \
{                                                                             \
   using Diluculum::Impl::CppObject;                                          \
                                                                              \
   CppObject* cppObj =                                                        \
      reinterpret_cast<CppObject*>(lua_touserdata (ls, -1));                  \
                                                                              \
   if (cppObj->deleteMe)                                                      \
   {                                                                          \
      cppObj->deleteMe = false; /* don't delete again when gc'ed! */          \
      CLASS* pObj = reinterpret_cast<CLASS*>(cppObj->ptr);                    \
      delete pObj;                                                            \
   }                                                                          \
                                                                              \
   return 0;                                                                  \
}



/** Exports a given class' method. This macro must be called between calls to
 *  \c DILUCULUM_BEGIN_CLASS() and \c DILUCULUM_END_CLASS(). Also, the method
 *  must have been previously wrapped by a call to \c DILUCULUM_WRAP_METHOD().
 *  @param CLASS The class whose method is being exported.
 *  @param METHOD The method being exported.
 */
#define DILUCULUM_CLASS_METHOD(CLASS, METHOD)                  \
   DILUCULUM_WRAP_METHOD(CLASS, METHOD);                       \
                                                               \
   namespace                                                   \
   {                                                           \
      Diluculum::Impl::ClassTableFiller                        \
         Diluculum__ ## CLASS ## _ ## METHOD ## __ ## Filler(  \
            DILUCULUM_CLASS_TABLE(CLASS),                      \
            #METHOD,                                           \
            DILUCULUM_METHOD_WRAPPER(CLASS, METHOD));          \
   }



/** Ends a block of class wrapping macro calls (which was opened by a call to
 *  \c DILUCULUM_BEGIN_CLASS()).
 *  @param CLASS The class being exported.
 */
#define DILUCULUM_END_CLASS(CLASS)                                      \
                                                                        \
/* The function used to register the class in a 'LuaState' */           \
void Diluculum_Register_Class__ ## CLASS (Diluculum::LuaState& ls)      \
{                                                                       \
   DILUCULUM_CLASS_TABLE(CLASS)["classname"] = #CLASS;                  \
                                                                        \
   DILUCULUM_CLASS_TABLE(CLASS)["new"] =                                \
      Diluculum__ ## CLASS ## __Constructor_Wrapper_Function;           \
                                                                        \
   DILUCULUM_CLASS_TABLE(CLASS)["delete"] =                             \
      Diluculum__ ## CLASS ## __Destructor_Wrapper_Function;            \
                                                                        \
   DILUCULUM_CLASS_TABLE(CLASS)["__gc"] =                               \
      Diluculum__ ## CLASS ## __Destructor_Wrapper_Function;            \
                                                                        \
   DILUCULUM_CLASS_TABLE(CLASS)["__index"] =                            \
      DILUCULUM_CLASS_TABLE(CLASS);                                     \
                                                                        \
   ls[#CLASS] = DILUCULUM_CLASS_TABLE(CLASS);                           \
} /* end of Diluculum_Register_Class__CLASS */



/** Registers a class in a given \c Diluculum::LuaState. The class must have
 *  been previously exported by calls to \c DILUCULUM_BEGIN_CLASS,
 *  \c DILUCULUM_END_CLASS() and probably \c DILUCULUM_CLASS_METHOD().
 *  @param LUA_STATE The \c Diluculum::LuaState in which the class will be
 *         available after this call.
 *  @param CLASS The class being registered.
 */
#define DILUCULUM_REGISTER_CLASS(LUA_STATE, CLASS)  \
   Diluculum_Register_Class__ ## CLASS (LUA_STATE);



/** Registers an object instantiated in C++ into a Lua state. This way, this
 *  object's methods can be called from Lua. The registered C++ object will
 *  \e not be destroyed when the corresponding Lua object is garbage-collected.
 *  Destroying it is responsibility of the programmer on the C++ side.
 *  @param LUA_VARIABLE The \c Diluculum::LuaVariable where the object will be
 *         stored. Notice that a \c Diluculum::LuaVariable contains a reference
 *         to a <tt>lua_State*</tt>, so the Lua state in which the object will
 *         be stored is passed here, too, albeit indirectly.
 *  @param CLASS The class of the object being registered. This class must have
 *         been previously registered in the target Lua state with a call to the
 *         \c DILUCULUM_REGISTER_CLASS() macro.
 *  @param OBJECT The object to be registered to the Lua state.
 *  @note Part of this macro's implementation is identical to the implementation
 *        of <tt>LuaVariable::operator=()</tt>. If someday the implementation
 *        here is replaced with something better, remember to change there, too.
 */
#define DILUCULUM_REGISTER_OBJECT(LUA_VARIABLE, CLASS, OBJECT)                 \
{                                                                              \
   /* leave the table where 'OBJECT' is to be stored at the stack top */       \
   lua_pushstring (LUA_VARIABLE.getState(), "_G");                             \
   lua_gettable (LUA_VARIABLE.getState(), LUA_GLOBALSINDEX);                   \
                                                                               \
   typedef Diluculum::LuaVariable::KeyList::const_iterator iter_t;             \
                                                                               \
   const Diluculum::LuaVariable::KeyList& keys = LUA_VARIABLE.getKeys();       \
                                                                               \
   assert (keys.size() > 0 && "At least one key should be present here.");     \
                                                                               \
   iter_t end = keys.end();                                                    \
      --end;                                                                   \
                                                                               \
   for (iter_t p = keys.begin(); p != end; ++p)                                \
   {                                                                           \
      PushLuaValue (LUA_VARIABLE.getState(), *p);                              \
      lua_gettable (LUA_VARIABLE.getState(), -2);                              \
      if (!lua_istable (LUA_VARIABLE.getState(), -1))                          \
      {                                                                        \
         throw Diluculum::TypeMismatchError(                                   \
            "table", luaL_typename (LUA_VARIABLE.getState(), -1));             \
      }                                                                        \
      lua_remove (LUA_VARIABLE.getState(), -2);                                \
   }                                                                           \
                                                                               \
   /* push the field where the object will be stored */                        \
   Diluculum::PushLuaValue (LUA_VARIABLE.getState(), keys.back());             \
                                                                               \
   /* create the userdata, set its metatable */                                \
   void* ud = lua_newuserdata (LUA_VARIABLE.getState(),                        \
                               sizeof(Diluculum::Impl::CppObject));            \
                                                                               \
   Diluculum::Impl::CppObject* cppObj =                                        \
      reinterpret_cast<Diluculum::Impl::CppObject*>(ud);                       \
                                                                               \
   cppObj->ptr = &OBJECT;                                                      \
   cppObj->deleteMe = false;                                                   \
                                                                               \
   lua_getglobal (LUA_VARIABLE.getState(), #CLASS);                            \
   lua_setmetatable (LUA_VARIABLE.getState(), -2);                             \
                                                                               \
   /* store the userdata */                                                    \
   lua_settable (LUA_VARIABLE.getState(), -3);                                 \
}



/** @todo Implement...
 */
#define DILUCULUM_BEGIN_MODULE(MODNAME)                  \
extern "C" int luaopen_ ## MODNAME (lua_State *luaState) \
{                                                        \
   using Diluculum::LuaState;                            \
   LuaState ls (luaState);



/** @todo Implement...
 */
#define DILUCULUM_MODULE_ADD_CLASS(CLASS)



/** @todo Implement...
 */
#define DILUCULUM_MODULE_ADD_FUNCTION(CFUNC, LUAFUNC)



/** @todo Implement...
 */
#define DILUCULUM_END_MODULE() \
   return 1;                   \
}

#endif // _DILUCULUM_LUA_WRAPPERS_HPP_
