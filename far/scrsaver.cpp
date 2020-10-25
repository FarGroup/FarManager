/*
scrsaver.cpp

ScreenSaver
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "scrsaver.hpp"

// Internal:
#include "palette.hpp"
#include "savescr.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "console.hpp"
#include "colormix.hpp"
#include "global.hpp"

// Platform:
#include "platform.chrono.hpp"

// Common:
#include "common/scope_exit.hpp"

// External:

//----------------------------------------------------------------------------

namespace
{
	class starfield
	{
		static const size_t StarsCount = 25;

		static const int
			Horizon = 1000,
			MaxSpeed = 25,
			Steps = 200;

		static const inline int Colours[]
		{
			F_BLUE,
			F_CYAN,
			F_RED,
			F_MAGENTA,
			F_BROWN
		};

		static const int DefaultColour = F_LIGHTGRAY;

		static const int DefaultColorPercentage = 95;

		static const int SteerPercentage = 33;

		static const inline wchar_t StarSprites[]
		{
			L'\x00B7', // ·
			L'\x2219', // ∙
			L'\x2022', // •
			L'\x25CF', // ●
		};

	public:
		NONCOPYABLE(starfield);

		starfield()
		{
			std::generate(ALL_RANGE(m_Stars), [&] { return create_star(); });

			SetScreen({ 0, 0, ScrX, ScrY }, L' ', colors::ConsoleColorToFarColor(F_LIGHTGRAY | B_BLACK));
		}

		void update()
		{
			if (m_Step != Steps)
			{
				++m_Step;
				m_Speed = MaxSpeed * factor();
			}
			else
			{
				if (m_SteerDirection == steer_direction::none)
				{
					if (m_SteerTime > 100)
					{
						if (m_PercentageDist(m_Engine) < SteerPercentage)
						{
							m_SteerAxis = static_cast<steer_axis>(m_SteerAxisDist(m_Engine));
							m_SteerDirection = static_cast<steer_direction>(m_SteerDirectionDist(m_Engine));
						}
						m_SteerTime = 0;
					}
				}
				else
				{
					if (m_SteerTime > 50)
					{
						m_SteerDirection = steer_direction::none;
						m_SteerTime = 0;
					}
				}
				++m_SteerTime;
			}

			for (auto& i: m_Stars)
			{
				const auto screen_position = [&]
				{
					const auto pos = [&](double const Value, int const Base)
					{
						return static_cast<int>((Value - Base / 2) * Horizon / i.Z + Base / 2);
					};

					return std::pair{ pos(i.X, ScrX), pos(i.Y, ScrY) };
				};

				SetColor(F_BLACK | B_BLACK);
				auto [X, Y] = screen_position();
				GotoXY(X, Y);
				Text(L' ');

				rotate(i);

				i.Z = std::max(0.0, i.Z - m_Speed);
				std::tie(X, Y) = screen_position();

				if (!i.Z || !in_closed_range(0, X, ScrX) || !in_closed_range(0, Y, ScrY))
				{
					i = create_star();
					std::tie(X, Y) = screen_position();
				}

				const size_t Size = (Horizon - i.Z) / static_cast<double>(Horizon * std::size(StarSprites));
				assert(Size < std::size(StarSprites));

				const auto Bright = Size >= std::size(StarSprites) / 2? FOREGROUND_INTENSITY : 0;
				SetColor(i.Color | Bright | B_BLACK);

				GotoXY(X, Y);
				Text(StarSprites[Size]);
			}
		}

	private:
		struct star
		{
			double X, Y, Z;
			int Color;
		};

		star create_star()
		{
			return
			{
				m_XDist(m_Engine),
				m_YDist(m_Engine),
				Horizon,
				m_PercentageDist(m_Engine) < DefaultColorPercentage? DefaultColour : Colours[m_ColorIndexDist(m_Engine)],
			};
		}

		void rotate(star& Star) const
		{
			if (m_SteerDirection == steer_direction::none)
				return;

			const auto
				XSteer = m_SteerAxis == steer_axis::X,
				YSteer = m_SteerAxis == steer_axis::Y,
				ZSteer = m_SteerAxis == steer_axis::Z;

			const auto
				LeftSteer = m_SteerDirection == steer_direction::left,
				RightSteer = m_SteerDirection == steer_direction::right;

			const auto AngleStep = ZSteer? 0.02 : 0.002;
			const auto Angle = LeftSteer? -AngleStep : RightSteer? +AngleStep : 0;

			const auto
				s = sin(Angle),
				c = cos(Angle);

			const auto
				Base1 = XSteer? 0 : ScrX / 2,
				Base2 = YSteer? 0 : ScrY / 2;

			auto
				&Coord1 = XSteer? Star.Z : Star.X,
				&Coord2 = YSteer? Star.Z : Star.Y;

			Coord1 -= Base1;
			Coord2 -= Base2;

			const auto IsY = &Coord1 == &Star.Y || &Coord2 == &Star.Y;

			if (IsY)
				Star.Y *= 2;

			auto
				New1 = Coord1 * c - Coord2 * s,
				New2 = Coord1 * s + Coord2 * c;

			if (IsY)
				(&Coord1 == &Star.Y? New1 : New2) /= 2;

			Coord1 = New1 + Base1;
			Coord2 = New2 + Base2;

		}

		double factor() const
		{
			// https://www.google.com/search?q=1%2F(1%2B(x%2F(1000-x))^-e)
			return 1 / (1 + std::pow(static_cast<double>(m_Step) / (Steps - m_Step), -std::exp(1)));
		}

		star m_Stars[StarsCount];

		std::uniform_real_distribution<>
			m_XDist{ 0, static_cast<double>(ScrX) },
			m_YDist{ 0, static_cast<double>(ScrY) };

		std::uniform_int_distribution<>
			m_PercentageDist{ 0, 99 },
			m_SteerAxisDist{ 0, static_cast<int>(steer_axis::count) - 1 },
			m_SteerDirectionDist{ 0, static_cast<int>(steer_direction::count) - 1 },
			m_ColorIndexDist{ 0, static_cast<int>(std::size(Colours) - 1) };

		std::mt19937 m_Engine{ static_cast<unsigned>(clock()) }; // std::random_device doesn't work in w2k

		double m_Speed{};
		int m_Step{};

		enum class steer_axis
		{
			X,
			Y,
			Z,

			count
		};

		enum class steer_direction
		{
			none,
			left,
			right,

			count
		};

		steer_axis m_SteerAxis{};
		steer_direction m_SteerDirection{};
		size_t m_SteerTime{};
	};
}

void ScreenSaver()
{
	static bool ScreenSaverActive = false;

	if (ScreenSaverActive)
		return;

	ScreenSaverActive = true;
	++Global->SuppressClock;

	SCOPE_EXIT
	{
		--Global->SuppressClock;
		ScreenSaverActive = false;
	};

	// The whole point of a screen saver is to be visible
	if (console.IsViewportShifted())
		console.ResetPosition();

	SCOPED_ACTION(SaveScreen);

	CONSOLE_CURSOR_INFO CursorInfo;
	console.GetCursorInfo(CursorInfo);
	SetCursorType(false, 10);

	SCOPE_EXIT
	{
		SetCursorType(CursorInfo.bVisible != FALSE, CursorInfo.dwSize);
	};

	starfield Starfield;

	INPUT_RECORD rec;
	while (!PeekInputRecord(&rec))
	{
		Starfield.update();
		os::chrono::sleep_for(25ms);
	}

	FlushInputBuffer();
	Global->StartIdleTime = std::chrono::steady_clock::now();
}
