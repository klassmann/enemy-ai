enemy_config = {
    speed = 2,
    cooldown = 0.5,
    max_distance = 200,
    follow_distance = 400 -- Distance to start chasing
}

-- Enemy states
STATE_IDLE = "IDLE"
STATE_CHASING = "CHASING"
STATE_RETURNING = "RETURNING"

enemy_state = {
    state = STATE_IDLE,
    cooldown_timer = 0
}

function update_enemy(player, enemy, home_x, home_y, delta_time)
    local distance_to_player = math.sqrt((player.x - enemy.x)^2 + (player.y - enemy.y)^2)
    local distance_to_home = math.sqrt((home_x - enemy.x)^2 + (home_y - enemy.y)^2)

    if enemy_state.state == STATE_IDLE then
        -- Transition to chasing if the player is within the follow distance
        local player_near = distance_to_player < enemy_config.follow_distance
        if player_near and enemy_state.cooldown_timer <= 0 then
            enemy_state.state = STATE_CHASING
        end

    elseif enemy_state.state == STATE_CHASING then
        -- Stop chasing and return if the player is too far
        if distance_to_player > enemy_config.max_distance then
            enemy_state.state = STATE_RETURNING
        end

        -- Chase the player
        return player.x, player.y

    elseif enemy_state.state == STATE_RETURNING then
        -- Transition back to IDLE if the enemy is close to home
        if distance_to_home < 5 then
            enemy_state.state = STATE_IDLE
            enemy_state.cooldown_timer = enemy_config.cooldown
        end

        -- Return to home position
        return home_x, home_y
    end

    -- Handle cooldown timer in IDLE state
    if enemy_state.state == STATE_IDLE and enemy_state.cooldown_timer > 0 then
        enemy_state.cooldown_timer = enemy_state.cooldown_timer - delta_time
    end

    -- Default: Stay in current position
    return enemy.x, enemy.y
end
