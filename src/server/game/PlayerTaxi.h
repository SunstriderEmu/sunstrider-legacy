#ifndef __PLAYERTAXI_H__
#define __PLAYERTAXI_H__

class TC_GAME_API PlayerTaxi
{
    public:
        PlayerTaxi();
        ~PlayerTaxi() = default;
        // Nodes
        void InitTaxiNodesForLevel(uint32 race, uint32 level);
        void LoadTaxiMask(const char* data);

        uint32 GetTaximask( uint8 index ) const { return m_taximask[index]; }
        bool IsTaximaskNodeKnown(uint32 nodeidx) const;
        void ResetTaximask() {
            for (auto& i : m_taximask)
                i = 0;
        }
        bool SetTaximaskNode(uint32 nodeidx)
        {
            uint8  field   = uint8((nodeidx - 1) / 32);
            uint32 submask = 1<<((nodeidx-1)%32);
            if ((m_taximask[field] & submask) != submask )
            {
                m_taximask[field] |= submask;
                return true;
            }
            else
                return false;
        }
        void AppendTaximaskTo(ByteBuffer& data,bool all);

        // Destinations
        bool LoadTaxiDestinationsFromString(const std::string& values, uint32 team);
        std::string SaveTaxiDestinationsToString();

        void ClearTaxiDestinations() { m_TaxiDestinations.clear(); }
        void AddTaxiDestination(uint32 dest) { m_TaxiDestinations.push_back(dest); }
        uint32 GetTaxiSource() const { return m_TaxiDestinations.empty() ? 0 : m_TaxiDestinations.front(); }
        uint32 GetTaxiDestination() const { return m_TaxiDestinations.size() < 2 ? 0 : m_TaxiDestinations[1]; }
        uint32 GetCurrentTaxiPath() const;
        uint32 NextTaxiDestination()
        {
            m_TaxiDestinations.pop_front();
            return GetTaxiDestination();
        }
        std::deque<uint32> const& GetPath() const { return m_TaxiDestinations; }
        bool empty() const { return m_TaxiDestinations.empty(); }
        FactionTemplateEntry const* GetFlightMasterFactionTemplate() const;
        void SetFlightMasterFactionTemplateId(uint32 factionTemplateId) { m_flightMasterFactionId = factionTemplateId; }

        friend std::ostringstream& operator<<(std::ostringstream& ss, PlayerTaxi const& taxi);
    private:
        TaxiMask m_taximask;
        std::deque<uint32> m_TaxiDestinations;
        uint32 m_flightMasterFactionId;
};

std::ostringstream& operator<<(std::ostringstream& ss, PlayerTaxi const& taxi);

#endif
