#pragma once

struct GlowObjectDefinition
{
	int						m_nNextFreeSlot;				// 0x00
	DWORD					m_pEntity;						// 0x04
	float					m_flRed;
	float					m_flGreen;
	float					m_flBlue;
	float					m_flAlpha;
	bool					m_bAlphaCappedByRenderAlpha;	// 0x18
	std::byte				pad0[0x3];						// 0x19 - pack 1 bool as 4 bytes
	float					m_flAlphaFunctionOfMaxVelocity;	// 0x1C
	float					m_flBloomAmount;				// 0x20
	float					m_flPulseOverdrive;				// 0x24
	bool					m_bRenderWhenOccluded;			// 0x28
	bool					m_bRenderWhenUnoccluded;		// 0x29
	bool					m_bFullBloomRender;				// 0x2A
	std::byte				pad1[0x1];						// 0x2B  - pack 3 bool as 4 bytes
	int						m_iFullBloomStencilTestValue;		// 0x2C
	int						m_nGlowStyle;					// 0x30
	int						m_nSplitScreenSlot;				// 0x34
};