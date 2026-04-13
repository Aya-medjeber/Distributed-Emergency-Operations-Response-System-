#pragma once

enum class ServerState
{
    NORMAL = 0,
    ALERT_ACTIVE,
    ESCALATED,
    RESOLVED
};