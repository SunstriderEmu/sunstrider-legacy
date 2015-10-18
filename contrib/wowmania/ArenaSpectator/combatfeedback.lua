local damage_format = "-%d"
local heal_format = "+%d"
local maxAlpha = 0.6
local updateFrame
local feedback = {}
local originalHeight = {}
local color 
local colors = {
	STANDARD		= { 1, 1, 1 }, -- color for everything not in the list below
	-- damage colors
	IMMUNE			= { 1, 1, 1 },
	DAMAGE			= { 1, 0, 0 },
	CRUSHING		= { 1, 0, 0 },
	CRITICAL		= { 1, 0, 0 },
	GLANCING		= { 1, 0, 0 },
	ABSORB			= { 1, 1, 1 },
	BLOCK			= { 1, 1, 1 },
	RESIST			= { 1, 1, 1 },
	MISS			= { 1, 1, 1 },
	-- heal colors
	HEAL			= { 0, 1, 0 },
	CRITHEAL		= { 0, 1, 0 },
	-- energize colors
	ENERGIZE		= { 0.41, 0.8, 0.94 },
	CRITENERGIZE	= { 0.41, 0.8, 0.94 },
}

local function createUpdateFrame()
	if updateFrame then return end
	updateFrame = CreateFrame("Frame")
	updateFrame:Hide()
	updateFrame:SetScript("OnUpdate", function()
		if next(feedback) == nil then
			updateFrame:Hide()
			return
		end
		for object, startTime in pairs(feedback) do
			local maxalpha = object.CombatFeedbackText.maxAlpha
			local elapsedTime = GetTime() - startTime
			if ( elapsedTime < COMBATFEEDBACK_FADEINTIME ) then
				local alpha = maxalpha*(elapsedTime / COMBATFEEDBACK_FADEINTIME)
				object.CombatFeedbackText:SetAlpha(alpha)
				object.CombatFeedbackText:SetTextHeight(math.abs(alpha*30)+1)
			elseif ( elapsedTime < (COMBATFEEDBACK_FADEINTIME + COMBATFEEDBACK_HOLDTIME) ) then
				object.CombatFeedbackText:SetAlpha(maxalpha)
				object.CombatFeedbackText:SetTextHeight(math.abs(maxalpha*30)+1)
			elseif ( elapsedTime < (COMBATFEEDBACK_FADEINTIME + COMBATFEEDBACK_HOLDTIME + COMBATFEEDBACK_FADEOUTTIME) ) then
				local alpha = maxalpha - maxalpha*((elapsedTime - COMBATFEEDBACK_HOLDTIME - COMBATFEEDBACK_FADEINTIME) / COMBATFEEDBACK_FADEOUTTIME)
				object.CombatFeedbackText:SetAlpha(alpha)
				object.CombatFeedbackText:SetTextHeight(math.abs(alpha*30)+1)
			else
				object.CombatFeedbackText:Hide()
				feedback[object] = nil
			end
		end		
	end)
end


local function combat(self, event, ...)
	local FeedbackText = self.CombatFeedbackText
	local fColors = FeedbackText.colors
	local font, fontHeight, fontFlags = FeedbackText:GetFont()
	fontHeight = FeedbackText.origHeight -- always start at original height
	local text, arg
	color = fColors and fColors.STANDARD or colors.STANDARD
	
	local timestamp, eventType = ...
	if eventType:find("SPELL") then
		timestamp, eventType, sourceGUID, sourceName, sourceFlags, destGUID, destName, destFlags, spellID, spellName, _, amount, _, resisted, blocked, absorbed, critical = ...
	elseif eventType:find("SWING") then
		timestamp, eventType, sourceGUID, sourceName, sourceFlags, destGUID, destName, destFlags, amount, _, resisted, blocked, absorbed, critical = ...
	end
	
	if destName ~= self.unit then return end
	
	if eventType:match("%w_DAMAGE") then
		if amount and amount > 0 then
			if critical then
				color = fColors and fColors.CRITICAL or colors.CRITICAL
				fontHeight = fontHeight * 1.5
			else
				color = fColors and fColors.DAMAGE or colors.DAMAGE
			end
			text = damage_format
			arg = amount
		elseif absorbed and absorbed > 0 then
			color = fColors and fColors.ABSORB or colors.ABSORB
			fontHeight = fontHeight * 0.75
			text = CombatFeedbackText["ABSORB"]
		elseif blocked then
			color = fColors and fColors.BLOCK or colors.BLOCK
			fontHeight = fontHeight * 0.75
			text = CombatFeedbackText["BLOCK"]
		elseif resisted then
			color = fColors and fColors.RESIST or colors.RESIST
			fontHeight = fontHeight * 0.75
			text = CombatFeedbackText["RESIST"]
		else
			color = fColors and fColors.MISS or colors.MISS
			text = CombatFeedbackText["MISS"]
		end
	elseif eventType:find("_HEAL") then
		if amount > 0 then
			text = heal_format
			arg = amount
			if critical then
				color = fColors and fColors.CRITHEAL or colors.CRITHEAL
				fontHeight = fontHeight * 1.3
			else
				color = fColors and fColors.HEAL or colors.HEAL
			end
		end
	end

	if text then
		FeedbackText:SetFont(font,fontHeight,fontFlags)
		FeedbackText:SetFormattedText(text, arg or 0)
		FeedbackText:SetTextColor(unpack(color))
		FeedbackText:SetAlpha(0)
		FeedbackText:Show()
		feedback[self] = GetTime()
		updateFrame:Show() -- start our onupdate
	end
end

function addCombatFeedback(object)
	if not object.CombatFeedbackText then return end
	-- store the original starting height
	local font, fontHeight, fontFlags = object.CombatFeedbackText:GetFont()
	object.CombatFeedbackText.origHeight = fontHeight
	object.CombatFeedbackText.maxAlpha = object.CombatFeedbackText.maxAlpha or maxAlpha
	createUpdateFrame()
	object:RegisterEvent("COMBAT_LOG_EVENT_UNFILTERED")
	if object:HasScript("OnEvent") then
		object:HookScript("OnEvent", function(self, event, ...)
			if event == "COMBAT_LOG_EVENT_UNFILTERED" then
				combat(self, event, ...)
			end
		end)
	else
		object:SetScript("OnEvent", function(self, event, ...)
			if event == "COMBAT_LOG_EVENT_UNFILTERED" then
				combat(self, event, ...)
			end
		end)
	end
end