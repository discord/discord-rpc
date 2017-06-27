

static DiscordEventHandlers Handlers;

void Discord_Initialize(const char* applicationId, DiscordEventHandlers* handlers)
{
    if (handlers) {
        memcpy(&Handlers, handlers, sizeof(Handlers));
    }
    else {
        memset(&Handlers, 0, sizeof(Handlers));
    }
}

void Discord_Shutdown()
{

}

void Discord_UpdatePresence(const DiscordRichPresence* presence)
{

}
