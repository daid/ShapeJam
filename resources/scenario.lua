
function run()
    message("Welcome new builder.\nI am here to guide you trough the first steps of becoming the production king.")
    confirm()
    message("Press and hold the right mouse button on the [{1}] on the ground to start collecting them. Do not worry, there is an endless supply.")

    objective("Collect 5x [{1}]")
    while countInventory("1") < 5 do yield() end
    objective("")
    
    message("Job well done.\nThe first steps are never easy. But you are doing great. Keep up the good work.")
    confirm()
    message("Now collect 10 [{3}] as well.")

    objective("Collect 10x [{3}]")
    while countInventory("3") < 10 do yield() end
    objective("")

    message("Excelent. Most people do not get this far. I think we have a winner here.")
    confirm()
    addInventory("SHAPER", 1)
    message("I have given you [{SHAPER}]. You can place this by selecting it on the left, and placing it anywhere on the map.")
    objective("Place the [{SHAPER}]")
    while countInventory("SHAPER") > 0 do yield() end
    objective("")
    message("Good. The [{SHAPER}] is a basic factory machine that is used to create other items.")
    confirm()
    message("Now place 3x [{3}] and 1x [{1}] on the [{SHAPER}] to have it fabricate 3x [{SQUARE3}]. And pickup the resulting [{SQUARE3}]")
    objective("Fabricate 3x [{SQUARE3}]")
    while countInventory("SQUARE3") < 3 do yield() end
    objective("")
    message("Eggcelent. While manually placing items in a machine like this can be done. But it is much more efficient to automate this. So let us do that now.")
    confirm()
    addInventory("MINER", 2)
    addInventory("BELT", 15)
    message("I have given you 2x [{MINER}] and a few [{BELT}]. These can be used to automate the whole process of creating [{SQUARE3}]")
    confirm()
    message("The [{MINER}] can be placed on items on the ground to extract them. And the [{BELT}] can then be used to move these into the [{SHAPER}].\nAlso, you can rotate any object by clicking on it again after selecting it.")
    objective("Automate [{SQUARE3}] production")
    --TOFIX: Check production stats
    while countInventory("SQUARE3") < 4 do yield() end

    message("Next we will need some [{SQUARE2}]. For for that, we will need to get [{2}]...")
    confirm()
    enableRotation()
    message("But, I have the solution for you. We'll add an extra dimension. I was told to just tell you to 'Click&Drag', whatever that may mean.")
    objective("Collect 3x [{2}]")
    while countInventory("2") < 3 do yield() end
    objective("")
    message("Good job. I'll give you some more machines to work with")
    confirm()
    addInventory("BELT", 10)
    addInventory("SPLITTER", 4)
    addInventory("BRIDGE", 4)
    addInventory("SHAPER", 1)
    addInventory("MINER", 2)
    message("You now also have access to [{SPLITTER}] and [{BRIDGE}]. These can be used to divide items across multiple [{BELT}], or cross atop of other [{BELT}].")
    confirm()
    message("")
    objective("Collect 3x [{SQUARE2}]")
    while countInventory("2") < 3 do yield() end
    objective("")
    message("We are getting near the end of me guiding you trough the first steps. There is one last thing.")
    confirm()
    message("I have given you a [{FACTORY}]. This can be used to create new machines. But be sure to set the production target on the right. Else it might produce something else then you want.")
    addInventory("FACTORY", 1)
    confirm()
    message("That is it for now. Have fun, and get your production stats up!")
    confirm()
    objective("Create 500x [{SQUARE2LINE3}]")
end
