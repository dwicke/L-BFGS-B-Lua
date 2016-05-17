
#include <lua.h>                               /* Always include this */
#include <lauxlib.h>                           /* Always include this */
#include <lualib.h>                            /* Always include this */
#include "lbfgsb.h"

static double f, g[1024];
static integer i__;
static double l[1024];
static integer m, n;
static double u[1024], x[1024], t1, t2, wa[43251];
static integer nbd[1024], iwa[3072];
/*     static char task[60]; */
static integer taskValue;
static integer *task=&taskValue; /* must initialize !! */
/*      http://stackoverflow.com/a/11278093/269192 */
static double factr;
/*     static char csave[60]; */
static integer csaveValue;
static integer *csave=&csaveValue;
static double dsave[29];
static integer isave[44];
static logical lsave[4];
static double pgtol;
static integer iprint;



int lua_setf(lua_State *L) {
  f = lua_tonumber(L, -1);
  //printf("f = %f", f);
  return 0;
}


int lua_setGrad(lua_State *L) {
  int len = lua_objlen(L, 1);
  int i =1;
  for ( i = 1; i <= len; ++i ) {
      lua_pushinteger( L, i );
      lua_gettable( L, -2 );
      g[i - 1] = lua_tonumber( L, -1 );
      lua_pop( L, 1 );
  }
  //printf("\ng[0] = %f", g[0]);
  //printf("\ng[1] = %f", g[1]);
  return 0;
}

int lua_setLowerBound(lua_State *L) {
  int len = lua_objlen(L, 1);
  int i  = 1;
  for ( i = 1; i <= len; ++i ) {
      lua_pushinteger( L, i );
      lua_gettable( L, -2 );
      l[i - 1] = lua_tonumber( L, -1 );
      lua_pop( L, 1 );
  }
  return 0;
}

int lua_setUpperBound(lua_State *L) {
  int len = lua_objlen(L, 1);
  int i = 1;
  for ( i = 1; i <= len; ++i ) {
      lua_pushinteger( L, i );
      lua_gettable( L, -2 );
      u[i - 1] = lua_tonumber( L, -1 );
      lua_pop( L, 1 );
  }
  return 0;
}

int lua_setNumVars(lua_State *L) {
  n = lua_tonumber(L, -1);
  return 0;
}






int lua_lbfgsb(lua_State *L) {
  *task = (integer)START;
  setulb(&n, &m, x, l, u, nbd, &f, g, &factr, &pgtol, wa, iwa, task, &
            iprint, csave, lsave, isave, dsave);

  return 1;
}


/**
  Need to have the new f and gradient values
*/
int lua_lbfgsbPulse(lua_State *L) {
  //printf("before x = %f , %f  f = %f  g = %f  %f task = %d \n", x[0], x[1], f, g[0], g[1], (*task));
  setulb(&n, &m, x, l, u, nbd, &f, g, &factr, &pgtol, wa, iwa, task, &
            iprint, csave, lsave, isave, dsave);
  //printf("after x = %f , %f  f = %f  g = %f  %f task = %d", x[0], x[1], f, g[0], g[1], (*task));
  if (!(*task==NEW_X) && !IS_FG(*task)) {
    lua_pushboolean(L, 1);
    return 1;
  }
  lua_pushboolean(L, 0);
  return 1;
}

// metatable method for handling "array[index]"
static int array_index (lua_State* L) {
   double** parray = luaL_checkudata(L, 1, "array");
   int index = luaL_checkint(L, 2);
   lua_pushnumber(L, (*parray)[index-1]);
   return 1;
}

// metatable method for handle "array[index] = value"
static int array_newindex (lua_State* L) {
   double** parray = luaL_checkudata(L, 1, "array");
   int index = luaL_checkint(L, 2);
   double value = luaL_checknumber(L, 3);
   (*parray)[index-1] = value;
   return 0;
}

// create a metatable for our array type
static void create_array_type(lua_State* L) {
   static const struct luaL_reg array[] = {
      { "__index",  array_index  },
      { "__newindex",  array_newindex  },
      NULL, NULL
   };
   luaL_newmetatable(L, "array");
   luaL_openlib(L, NULL, array, 0);
}

// expose an array to lua, by storing it in a userdata with the array metatable
static int expose_array(lua_State* L, double array[]) {
   double** parray = lua_newuserdata(L, sizeof(double**));
   *parray = array;
   luaL_getmetatable(L, "array");
   lua_setmetatable(L, -2);
   return 1;
}



// test routine which exposes our test array to Lua
static int getInputArray (lua_State* L) {
   return expose_array( L, x );
}

int luaopen_luawrapper(lua_State *L){
  m = 15; // number of corrections  should be 3 <= m <= 20 (larger is slower)
  iprint = -1; // < 0 for no printing
  pgtol = 1e-10;
  factr = 1e7; //

  int i = 0;
  for (i = 0; i < 1024; i++) {
    nbd[i] = 2; // set to say both lower and upper bounded for all variables
  }

  create_array_type(L);

  lua_register(L, "startLBFGSB", lua_lbfgsb);
  lua_register(L, "pulseLBFGSB", lua_lbfgsbPulse);
  lua_register(L, "getInputArray", getInputArray);
  lua_register(L, "setf", lua_setf);
  lua_register(L, "setGrad", lua_setGrad);
  lua_register(L, "setUpperBound", lua_setUpperBound);
  lua_register(L, "setLowerBound", lua_setLowerBound);
  lua_register(L, "setNumberVars", lua_setNumVars);
  return 0;
}
