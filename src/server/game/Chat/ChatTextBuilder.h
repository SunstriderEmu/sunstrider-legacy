
#ifndef __CHATTEXT_BUILDER_H
#define __CHATTEXT_BUILDER_H

#include "Chat.h"
#include "ObjectMgr.h"

namespace Trinity
{
    class BroadcastTextBuilder
    {
        public:
            BroadcastTextBuilder(Unit const* obj, ChatMsg msgType, uint32 textId, WorldObject const* target = nullptr, uint32 achievementId = 0)
                : _source(obj), _msgType(msgType), _textId(textId), _target(target), _achievementId(achievementId) { }

            void operator()(WorldPacket& data, LocaleConstant locale)
            {
                BroadcastText const* bct = sObjectMgr->GetBroadcastText(_textId);
                ChatHandler::BuildChatPacket(data, _msgType, bct ? Language(bct->Language) : LANG_UNIVERSAL, _source, _target, bct ? bct->GetText(locale, _source->GetGender()) : "", _achievementId, "", locale);
            }

            size_t operator()(WorldPacket* data, LocaleConstant locale) const
            {
                BroadcastText const* bct = sObjectMgr->GetBroadcastText(_textId);
                return ChatHandler::BuildChatPacket(*data, _msgType, bct ? Language(bct->Language) : LANG_UNIVERSAL, _source, _target, bct ? bct->GetText(locale, _source->GetGender()) : "", _achievementId, "", locale);
            }

        private:
            Unit const* _source;
            ChatMsg _msgType;
            uint32 _textId;
            WorldObject const* _target;
            uint32 _achievementId;
    };

    class CustomChatTextBuilder
    {
        public:
            CustomChatTextBuilder(WorldObject const* obj, ChatMsg msgType, std::string  text, Language language = LANG_UNIVERSAL, WorldObject const* target = nullptr)
                : _source(obj), _msgType(msgType), _text(std::move(text)), _language(language), _target(target) { }

            void operator()(WorldPacket& data, LocaleConstant locale)
            {
                ChatHandler::BuildChatPacket(data, _msgType, _language, _source, _target, _text, 0, "", locale);
            }

        private:
            WorldObject const* _source;
            ChatMsg _msgType;
            std::string _text;
            Language _language;
            WorldObject const* _target;
    };

    //Do not use, only for retrocompat. Build from script_text and trinity_string
    class OldScriptTextBuilder
    {
        public:
            OldScriptTextBuilder(Unit const* obj, ChatMsg msgType, uint32 textId, Language lang = LANG_UNIVERSAL, WorldObject const* target = nullptr)
                : _source(obj), _msgType(msgType), _textId(textId), _target(target), _lang(lang) { }

            void operator()(WorldPacket& data, LocaleConstant locale)
            {
                char const* text = sObjectMgr->GetTrinityString(_textId,locale);

                ChatHandler::BuildChatPacket(data, _msgType, _lang, _source, _target, text, 0, "", locale);
            }

            size_t operator()(WorldPacket* data, LocaleConstant locale) const
            {
                char const* text = sObjectMgr->GetTrinityString(_textId,locale);

                return ChatHandler::BuildChatPacket(*data, _msgType, _lang, _source, _target, text, 0, "", locale);
            }

        private:
            Unit const* _source;
            ChatMsg _msgType;
            uint32 _textId;
            WorldObject const* _target;
            Language _lang;
    };

}
// namespace Trinity

#endif // __CHATTEXT_BUILDER_H
