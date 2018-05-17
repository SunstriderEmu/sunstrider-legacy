#ifndef __UPDATEMASK_H
#define __UPDATEMASK_H

#include "ByteBuffer.h"

class UpdateMask
{
    public:
        /// Type representing how client reads update mask
        typedef uint32 ClientUpdateMaskType;

        enum UpdateMaskCount
        {
            CLIENT_UPDATE_MASK_BITS = sizeof(ClientUpdateMaskType) * 8,
        };

        UpdateMask() : _fieldCount(0), _blockCount(0), _bits(nullptr) { }

        UpdateMask(UpdateMask const& right) : _bits(nullptr)
        {
            SetCount(right.GetCount());
            memcpy(_bits, right._bits, sizeof(uint8) * _blockCount * 32);
        }

        ~UpdateMask() { delete[] _bits; }

        void SetBit(uint32 index, bool set = true) { _bits[index] = uint8(set); }
        bool GetBit(uint32 index) const { return _bits[index] != 0; }

        void AppendToPacket(ByteBuffer* data)
        {
            for (uint32 i = 0; i < GetBlockCount(); ++i)
            {
                ClientUpdateMaskType maskPart = 0;
                for (uint32 j = 0; j < CLIENT_UPDATE_MASK_BITS; ++j)
                    if (_bits[CLIENT_UPDATE_MASK_BITS * i + j])
                        maskPart |= 1 << j;

                *data << maskPart;
            }
        }

        uint32 GetBlockCount() const { return _blockCount; }
        uint32 GetCount() const { return _fieldCount; }

        void SetCount(uint32 valuesCount)
        {
            delete[] _bits;

            _fieldCount = valuesCount;
            _blockCount = (valuesCount + CLIENT_UPDATE_MASK_BITS - 1) / CLIENT_UPDATE_MASK_BITS;

            _bits = new uint8[_blockCount * CLIENT_UPDATE_MASK_BITS];
            memset(_bits, 0, sizeof(uint8) * _blockCount * CLIENT_UPDATE_MASK_BITS);
        }

        void Clear()
        {
            if (_bits)
                memset(_bits, 0, sizeof(uint8) * _blockCount * CLIENT_UPDATE_MASK_BITS);
        }

        UpdateMask& operator=(UpdateMask const& right)
        {
            if (this == &right)
                return *this;

            SetCount(right.GetCount());
            memcpy(_bits, right._bits, sizeof(uint8) * _blockCount * CLIENT_UPDATE_MASK_BITS);
            return *this;
        }

        UpdateMask& operator&=(UpdateMask const& right)
        {
            ASSERT(right.GetCount() <= GetCount());
            for (uint32 i = 0; i < _fieldCount; ++i)
                _bits[i] &= right._bits[i];

            return *this;
        }

        UpdateMask& operator|=(UpdateMask const& right)
        {
            ASSERT(right.GetCount() <= GetCount());
            for (uint32 i = 0; i < _fieldCount; ++i)
                _bits[i] |= right._bits[i];

            return *this;
        }

        UpdateMask operator|(UpdateMask const& right)
        {
            UpdateMask ret(*this);
            ret |= right;
            return ret;
        }

    private:
        /** Total update field count for object, updated or not */
        uint32 _fieldCount;
        /** Or 'how much uint32 blocks do we need to fit one bit per field' */
        uint32 _blockCount; 
        /* Complete update mask, one bit per field */
        uint8* _bits;
};

#endif

