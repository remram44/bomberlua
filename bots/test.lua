function can_go(direction, bomber, map)
    if direction == "right"
      and (map[(bomber.posx + 1) .. "," .. bomber.posy] == "empty") then
        return true
    elseif direction == "up"
      and (map[bomber.posx .. "," .. (bomber.posy - 1)] == "empty") then
        return true
    elseif direction == "left"
      and (map[(bomber.posx - 1) .. "," .. bomber.posy] == "empty") then
        return true
    elseif direction == "down"
      and (map[bomber.posx .. "," .. (bomber.posy + 1)] == "empty") then
        return true
    else
        return false
    end
end

function next_dir(direction, wall)
    if wall == "right" then
        if direction == "right" then return "down"
        elseif direction == "up" then return "right"
        elseif direction == "left" then return "up"
        elseif direction == "down" then return "left"
        else
            return "right"
        end
    elseif wall == "left" then
        if direction == "right" then return "up"
        elseif direction == "up" then return "left"
        elseif direction == "left" then return "down"
        elseif direction == "down" then return "right"
        else
            return "right"
        end
    end
end

function invert_wall(wall)
    if wall == "right" then return "left"
    elseif wall == "left" then return "right"
    end
end

wall = "left"

ready()

-- boucle de jeu
while 1 do
    map = get_field()
    bomber = get_self()

    -- on choisit une direction de telle façon que le bomber suive le mur situé
    -- à sa gauche
    direction = next_dir(direction, invert_wall(wall))
    while not can_go(direction, bomber, map) do
        direction = next_dir(direction, wall)
    end

    -- on va dans la direction choisie
    move(direction)

    -- une chance sur 5 de poser une bombe
    if math.random(1, 5) == 1 then
        bomb()
    end

    -- on a une chance sur 5 de changer de mur à suivre
    if math.random(1, 5) == 1 then
        wall = invert_wall(wall)
    end
end
