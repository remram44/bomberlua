--------------------------------------------------------------------------------
-- remram44.lua - bot by remram44.
-- This schizophrenic bot has two different behaviors (offensive and defensive).
-- The combination of those two determines the action to take.
--

--------------------------------------------------------------------------------
-- Initialization

local debug = true
--local debug = true
if not debug then
    log = function() end
end

-- moves is an array containing the possible actions. It is shuffled each turn
-- to make decisions more random.
moves = {"right", "up", "left", "down", "wait"}
-- The moves associated to each action
dirs = {right = {x = 1, y = 0}, up = {x = 0, y = -1}, left = {x = -1, y = 0},
    down = {x = 0, y = 1}, wait = {x = 0, y = 0}}

-- Function to shuffle an array (returns a new array)
function table.random(t)
    local result = {}
    local len = table.getn(t)

    -- Loop on the input array
    for i = 1, len do
        -- Choose the index in the output array, amongst those that are not yet
        -- filled
        local index = math.random(1, len - i + 1)
        local j = 0
        while index > 0 do
            j = j + 1
            if result[j] == nil then
                index = index - 1
            end
        end
        -- Insert
        result[j] = t[i]
    end

    return result
end

-- Function to check whether we can hide from a given bomb
-- x, y is the position of a bomb, n its range in cells
function can_escape(field, x, y, n)
    log("Can we escape from a bomb planted at (" .. x .. ";" .. y .. ") with "
        .. "range " .. n .. "?")

    -- Loop on all 4 directions from that bomb
    for k, dir in pairs(dirs) do
        if k == "wait" then break end

        log("  Checking direction: " .. k)

        local x2, y2 = x, y

        -- Move forward in that direction while the bomb has more range
        for i = n, 0, -1 do
            x2 = x2 + dir.x
            y2 = y2 + dir.y

            log("  Position : (" .. x2 .. ";" .. y2 .. ")")

            -- If we reached bomb's range
            if n == 0 then
                if field[x2 .. "," .. y2] == "empty" then
                    log("  Yes: by going to (" .. x2 .. ";" .. y2 .. ") "
                        .. "(out of range)")
                    return true
                end
                break
            end

            -- If we reached a wall, no need to go on
            if field[x2 .. "," .. y2] ~= "empty" then
                break
            end

            -- Look in directions other than the explosion's to find hiding
            -- places
            if dir.x == 0 and field[(x2 - 1) .. "," .. y2] == "empty" then
                log("  Yes: by moving to (" .. (x2 - 1) .. ";" .. y2
                    .. ") (hidden)")
                return true
            end
            if dir.x == 0 and field[(x2 + 1) .. "," .. y2] == "empty" then
                log("  Yes: by moving to (" .. (x2 + 1) .. ";" .. y2
                    .. ") (hidden)")
                return true
            end
            if dir.y == 0 and field[x2 .. "," .. (y2 - 1)] == "empty" then
                log("  Yes: by moving to (" .. x2 .. ";" .. (y2 - 1)
                    .. ") (hidden)")
                return true
            end
            if dir.y == 0 and field[x2 .. "," .. (y2 + 1)] == "empty" then
                log("  Yes: by moving to (" .. x2 .. ";" .. (y2 + 1)
                    .. ") (hidden)")
                return true
            end
        end -- If we didn't find any, move forward
    end -- Still haven't found any, next direction of explosion

    -- If we didn't find anything, planting a bomb at that position is not a
    -- good idea since we wouldn't be able to escape it
    log("  No! :(")
    return false
end

-- Offensive behavior
function attack(self, field, bombers)
    local scores = {}

    -- Choose a spot to plant a bomb
    -- The best spot is near an enemy, else near destructible walls.
    -- The chosen spot needs to leave us a chance to hide from the bomb.

    -- Loop in possible directions
    for k, dir in pairs(dirs) do
        local x, y = self.posx + dir.x, self.posy + dir.y

        scores[k] = 0

        -- Occupied cell: can't move there
        if field[x .. "," .. y] ~= "empty" then
            log("attack(): cell towards \"" .. k .. "\" occupied (\""
                .. field[x .. "," .. y] .. "\")")
            scores[k] = -100

        -- Else
        else
            for k2, dir2 in pairs(dirs) do
                local x2, y2 = x + dir2.x, y + dir2.y
                -- If there is a destructible wall
                if field[x2 .. "," .. y2] == "brick" then
                    scores[k] = scores[k] + 0.5
                -- If there's nothing
                elseif field[x2 .. "," .. y2] == "empty" then
                    if k ~= "wait" then
                        scores[k] = scores[k] + 0.2
                    else
                        scores[k] = scores[k] + 0.1
                    end
                -- If there's a wall
                elseif field[x2 .. "," .. y2] == "rock" then
                    if k == "wait" then
                        scores[k] = scores[k] + 0.1
                    end
                -- If there is a bomb, 0
                -- If there is an explosion
                elseif field[x2 .. "," .. y2] == "explosion" then
                    scores[k] = scores[k] - 1
                end

                -- TODO: attack enemies
            end
        end
        log("attack() : scores[" .. k .. "] = " .. scores[k])
    end

    -- Should we plant a bomb?
    local drop
    -- We can plant a bomb only if the score of this spot is enough, and if we
    -- can escape it
    if scores["wait"] < 0.7 then
        drop = false
    else
        drop = can_escape(field, self.posx, self.posy, 1)
    end

    -- Waiting is bad, only do it of other cells are dangerous (negative score)
    scores["wait"] = 0.1

    if drop then
        log("BOMB!")
    end

    return scores, drop
end

-- Defensive behavior
function defend(self, field)
    local scores = {}
    
    -- Loop on possible directions
    for k, dir in pairs(dirs) do
        local x, y = self.posx + dir.x, self.posy + dir.y

        -- If there are bombs near this cell, decrement its score by taking the
        -- remaining time before explosion into account, and increment it for
        -- each possible escape paths.
        -- If there is no bomb nearby, the defensive score is 0.
        scores[k] = 0
        for k2, dir2 in pairs(dirs) do
            local x2, y2 = x + dir2.x, y + dir2.y
            -- If there is a bomb
            local t = tonumber(field[x2 .. "," .. y2])
            if t ~= nil then
                log("defend(): nearby bomb! Score: "
                    .. ((-10/(t+0.5)) - 2))
                scores[k] = scores[k] - (10/(t+0.5)) - 2
            end
        end
        -- Take the empty cells into account
        if scores[k] < 0 then
            for k2, dir2 in pairs(dirs) do
                local x2, y2 = x + dir2.x, y + dir2.y
                if field[x2 .. "," .. y2] == "empty" then
                    scores[k] = scores[k] + 0.5
                end
            end
        end
    end

    -- Special case: there is a bomb where we are
    if tonumber(field[self.posx .. "," .. self.posy]) ~= nil then
        scores["wait"] = -100 -- can't stay here
    end

    return scores
end

-- End of initialization
ready()
--------------------------------------------------------------------------------

-- Main loop
while 1 do
    log("Loop...")

    local field = get_field()
    local self = get_self()
    local bombers = get_bombers()

    -- Offensive behavior: compute the score of each direction with regard to
    -- planting bombs
    local att, drop = attack(self, field, bombers)

    if drop then
        bomb()
        field = get_field()
    end

    -- Defensive behavior: compute the score of each direction with regard to
    -- avoiding bombs
    local def = defend(self, field)

    -- Add those up
    local scores = {
        right = att.right + def.right, up = att.up + def.up,
        left = att.left + def.left, down = att.down + def.down,
        wait = att.wait + def.wait}

    -- The best direction is chosen
    moves = table.random(moves)
    best = 1
    for i = 2, 5 do
        if scores[moves[i]] > scores[moves[best]] then
            best = i
        end
    end
    log("scores: right = " .. scores.right .. ", up = " .. scores.up
        .. ", left = " .. scores.left .. ", down = " .. scores.down
        .. ", wait = " .. scores.wait, " choice: " .. moves[best])
    if moves[best] ~= "wait" then
        move(moves[best])
    end
end
