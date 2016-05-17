package.path = package.path..";../../?/init.lua"
package.path = package.path..";../../?.lua"

require "luawrapper"


function getf(action1, action2)
  local x1,y1 = -10,-10
  local h1 = 100
  local s1 = 4.0
  local f1 = h1*(1 - (((action1-x1)/s1)*((action1-x1)/s1)) - (((action2-y1)/s1)*((action2-y1)/s1))) + 50
  print ("lua f = ".. f1 .. "  actions  = " .. action1 .. " " .. action2)
  return -f1
end

function getgrad(action1, action2)
  local x1,y1 = -10,-10
  local h1 = 100
  local s1 = 4.0
  local f1 = { 2*h1* ((action1-x1)/(s1*s1)), 2*h1* ((action2-y1)/(s1*s1)) }
  return f1
end

function init()
  local upperB = {0, 0}
  local lowerB = {-20,-20}
  x = getInputArray()
  x[1] = 0.1
  x[2] = 4
  setNumberVars(2)
  setUpperBound(upperB)
  setLowerBound(lowerB)
  setf(getf(x[1], x[2]))
  setGrad(getgrad(x[1], x[2]))
  startLBFGSB()
end

function pulse()
  setf(getf(x[1], x[2]))
  setGrad(getgrad(x[1], x[2]))
  return pulseLBFGSB()
end

function main()
  init()
  count = 0
  print("count = " .. count)


  while pulse() == false  do
    print("final x vals = " .. x[1] .. " " .. x[2])
    print("the count is " .. count)
    count = count + 1
  end

end

main()
