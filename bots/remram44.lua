--------------------------------------------------------------------------------
-- remram44.lua - bot par remram44.
-- Ce bot a deux comportements (offensif et défensif) distincts. La combinaison
-- des choix de ces deux parties décide de l'action à effectuer.
--

--------------------------------------------------------------------------------
-- Initialisation

local debug = true
--local debug = true
if not debug then
    log = function() end
end

-- moves est un tableau contenant les choix d'action possibles. Il est mélangé
-- à chaque tour de jeu afin de permettre des décisions plus aléatoires.
moves = {"right", "up", "left", "down", "wait"}
-- Les mouvements associés à chaque action
dirs = {right = {x = 1, y = 0}, up = {x = 0, y = -1}, left = {x = -1, y = 0},
    down = {x = 0, y = 1}, wait = {x = 0, y = 0}}

-- Fonction permettant de mélanger un tableau (retourne un nouveau tableau)
function table.random(t)
    local result = {}
    local len = table.getn(t)

    -- On parcourt tout le tableau de départ
    for i = 1, len do
        -- On choisi l'indice dans le tableau d'arrivée, parmi ceux qui ne sont
        -- pas encore utilisés
        local index = math.random(1, len - i + 1)
        local j = 0
        while index > 0 do
            j = j + 1
            if result[j] == nil then
                index = index - 1
            end
        end
        -- On fait l'insertion
        result[j] = t[i]
    end

    return result
end

-- Fonction permettant de déterminer si on peut se mettre à l'abris d'une bombe
-- donnée
-- x, y est la position de la bombe, n sa portée en cases
function can_escape(field, x, y, n)
    log("Peut-on echapper a une bombe posee en (" .. x .. ";" .. y .. ") de "
        .. "portee " .. n .. " ?")

    -- On boucle sur les 4 directions partant de cette bombe
    for k, dir in pairs(dirs) do
        if k == "wait" then break end

        log("  Direction examinee : " .. k)

        local x2, y2 = x, y

        -- On avance dans cette direction tant que la bombe a de la portée
        for i = n, 0, -1 do
            x2 = x2 + dir.x
            y2 = y2 + dir.y

            log("  Position : (" .. x2 .. ";" .. y2 .. ")")

            -- Si on a dépassé la portée de la bombe
            if n == 0 then
                if field[x2 .. "," .. y2] == "empty" then
                    log("  Oui : en se placant en (" .. x2 .. ";" .. y2 .. ") "
                        .. "(hors de portee)")
                    return true
                end
                break
            end

            -- Si on atteint un mur, inutile de continuer
            if field[x2 .. "," .. y2] ~= "empty" then
                break
            end

            -- On regarde dans les directions autres que celles de l'explosion
            -- pour trouver un endroit où se cacher
            if dir.x == 0 and field[(x2 - 1) .. "," .. y2] == "empty" then
                log("  Oui : en se placant en (" .. (x2 - 1) .. ";" .. y2
                    .. ") (cache)")
                return true
            end
            if dir.x == 0 and field[(x2 + 1) .. "," .. y2] == "empty" then
                log("  Oui : en se placant en (" .. (x2 + 1) .. ";" .. y2
                    .. ") (cache)")
                return true
            end
            if dir.y == 0 and field[x2 .. "," .. (y2 - 1)] == "empty" then
                log("  Oui : en se placant en (" .. x2 .. ";" .. (y2 - 1)
                    .. ") (cache)")
                return true
            end
            if dir.y == 0 and field[x2 .. "," .. (y2 + 1)] == "empty" then
                log("  Oui : en se placant en (" .. x2 .. ";" .. (y2 + 1)
                    .. ") (cache)")
                return true
            end
        end -- Si on en a pas trouvé, on avance
    end -- Si on en a toujours pas trouvé, direction suivante

    -- Si on en a pas trouvé du tout, poser une bombe à cet endroit n'est pas
    -- une bonne idée car il serait impossible de s'en cacher
    log("  Non ! :(")
    return false
end

-- Comportement offensif
function attack(self, field, bombers)
    local scores = {}

    -- Choix d'un endroit pour aller poser une bombe
    -- Le mieux est un endroit proche d'un ennemi, sinon proche de blocs
    -- destructibles.
    -- L'endroit choisi doit laisser une chance de se cacher de la bombe.

    -- On boucle sur les directions possibles
    for k, dir in pairs(dirs) do
        local x, y = self.posx + dir.x, self.posy + dir.y

        scores[k] = 0

        -- Case occupée : impossible de s'y rendre
        if field[x .. "," .. y] ~= "empty" then
            log("attack() : case vers \"" .. k .. "\" occupee (\""
                .. field[x .. "," .. y] .. "\")")
            scores[k] = -100

        -- Sinon
        else
            for k2, dir2 in pairs(dirs) do
                local x2, y2 = x + dir2.x, y + dir2.y
                -- S'il y a un bloc destructible
                if field[x2 .. "," .. y2] == "brick" then
                    scores[k] = scores[k] + 0.5
                -- S'il n'y a rien
                elseif field[x2 .. "," .. y2] == "empty" then
                    if k ~= "wait" then
                        scores[k] = scores[k] + 0.2
                    else
                        scores[k] = scores[k] + 0.1
                    end
                -- S'il y a un mur
                elseif field[x2 .. "," .. y2] == "rock" then
                    if k == "wait" then
                        scores[k] = scores[k] + 0.1
                    end
                -- S'il y a une bombe, 0
                -- S'il y a une explosion
                elseif field[x2 .. "," .. y2] == "explosion" then
                    scores[k] = scores[k] - 1
                end

                -- TODO : attaque des ennemis
            end
        end
        log("attack() : scores[" .. k .. "] = " .. scores[k])
    end

    -- Doit-on poser une bombe ?
    local drop
    -- On peut poser une bombe uniquement si le score de cette case est
    -- suffisant, et si on a un endroit où se cacher
    if scores["wait"] < 0.7 then
        drop = false
    else
        drop = can_escape(field, self.posx, self.posy, 1)
    end

    -- Attendre c'est mal, on le fait que si les autres cases sont dangereuses
    -- (score négatif)
    scores["wait"] = 0.1

    if drop then
        log("BOMB !")
    end

    return scores, drop
end

-- Comportement défensif
function defend(self, field)
    local scores = {}
    
    -- On boucle sur les directions possibles
    for k, dir in pairs(dirs) do
        local x, y = self.posx + dir.x, self.posy + dir.y

        -- S'il y a des bombes à proximité de la case, on diminue son score en
        -- prenant en compte le temps restant avant explosion, et on l'augmente
        -- pour chaque passage libre.
        -- S'il n'y a pas de bombe à proximité, le score de défense est 0.
        scores[k] = 0
        for k2, dir2 in pairs(dirs) do
            local x2, y2 = x + dir2.x, y + dir2.y
            -- S'il y a une bombe
            local t = tonumber(field[x2 .. "," .. y2])
            if t ~= nil then
                log("defend() : bombe a proximite ! Score : "
                    .. ((-10/(t+0.5)) - 2))
                scores[k] = scores[k] - (10/(t+0.5)) - 2
            end
        end
        -- Prise en compte des cases libres
        if scores[k] < 0 then
            for k2, dir2 in pairs(dirs) do
                local x2, y2 = x + dir2.x, y + dir2.y
                if field[x2 .. "," .. y2] == "empty" then
                    scores[k] = scores[k] + 0.5
                end
            end
        end
    end

    -- Cas spécial : il y a une bombe à l'endroit où on est
    if tonumber(field[self.posx .. "," .. self.posy]) ~= nil then
        scores["wait"] = -100 -- impossible de rester ici
    end

    return scores
end

-- Fin de l'initialisation
ready()
--------------------------------------------------------------------------------

-- Boucle principale
while 1 do
    log("Boucle...")

    local field = get_field()
    local self = get_self()
    local bombers = get_bombers()

    -- Comportement offensif : on détermine le score de chaque direction dans le
    -- but d'aller poser des bombes
    local att, drop = attack(self, field, bombers)

    if drop then
        bomb()
        field = get_field()
    end

    -- Comportement défensif : on détermine le score de chaque direction dans le
    -- but d'éviter les bombes
    local def = defend(self, field)

    -- On ajoute tout ça
    local scores = {
        right = att.right + def.right, up = att.up + def.up,
        left = att.left + def.left, down = att.down + def.down,
        wait = att.wait + def.wait}

    -- La direction qui a le meilleur score est choisie
    moves = table.random(moves)
    best = 1
    for i = 2, 5 do
        if scores[moves[i]] > scores[moves[best]] then
            best = i
        end
    end
    log("scores : right = " .. scores.right .. ", up = " .. scores.up
        .. ", left = " .. scores.left .. ", down = " .. scores.down
        .. ", wait = " .. scores.wait, " choix : " .. moves[best])
    if moves[best] ~= "wait" then
        move(moves[best])
    end
end
