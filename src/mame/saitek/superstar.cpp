// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

SciSys Superstar / Turbostar
Starting from Turbostar 432, SciSys started adding the "Kasparov" prefix.

Hardware notes (Superstar 28K):
- PCB label: YO1C-PE-017 REV2
- R6502AP @ ~2MHz (no XTAL, clock from RC circuit?)
- 4KB RAM (2*HM6116P-4)
- 24KB ROM (3*M5L2764K)
- TTL, buzzer, 28 LEDs, 8*8 chessboard buttons

Superstar 36K:
- PCB label: YO1CD-PE-006 REV3
- SYU6502A @ 2MHz
- 4KB RAM (2*HM6116P-3)
- 32KB ROM (custom label), extension ROM slot

Turbostar 432:
- PCB label: SUPERSTAR REV-3
- R65C02P4 @ 4MHz
- 4KB RAM (2*HM6116P-4)
- 32KB ROM (custom label, contents nearly identical to sstar36k)
- extension ROM slot

There are 2 versions of Turbostar 432, the 2nd one has a lighter shade and
the top-right is gray instead of red. It came with the KSO ROM included.

I.C.D. (a reseller in USA, NY) also sold an overclocked version (first 5MHz,
and later 5.53MHz), and named it Turbostar 540 Plus. The ROM is unmodified,
so the internal chess clock would run too fast.

TODO:
- verify sstar28k CPU speed

******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6502/r65c02.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "saitek_sstar28k.lh" // clickable
#include "saitek_tstar432.lh" // clickable


namespace {

class star_state : public driver_device
{
public:
	star_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine configs
	void sstar28k(machine_config &config);
	void sstar36k(machine_config &config);
	void tstar432(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<2> m_inputs;

	// address maps
	void sstar28k_map(address_map &map);
	void tstar432_map(address_map &map);

	// I/O handlers
	void control_w(u8 data);
	u8 input_r();

	u8 m_inp_mux = 0;
};

void star_state::machine_start()
{
	save_item(NAME(m_inp_mux));
}



/******************************************************************************
    I/O
******************************************************************************/

void star_state::control_w(u8 data)
{
	// d0-d3: input mux, led select
	m_inp_mux = data & 0xf;

	// d4-d6: led data
	m_display->matrix(1 << m_inp_mux, ~data >> 4 & 7);

	// d7: speaker out
	m_dac->write(BIT(data, 7));
}

u8 star_state::input_r()
{
	u8 data = 0;

	// d0-d7: multiplexed inputs
	// read chessboard sensors
	if (m_inp_mux < 8)
		data = m_board->read_file(m_inp_mux);

	// read other buttons
	else if (m_inp_mux < 10)
		data = m_inputs[m_inp_mux - 8]->read();

	return data;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void star_state::sstar28k_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0x2000, 0x27ff).mirror(0x1800).ram();
	map(0x4000, 0x4000).w(FUNC(star_state::control_w));
	map(0x6000, 0x6000).r(FUNC(star_state::input_r));
	map(0xa000, 0xffff).rom();
}

void star_state::tstar432_map(address_map &map)
{
	sstar28k_map(map);

	map(0x4000, 0x5fff).r("extrom", FUNC(generic_slot_device::read_rom));
	map(0x8000, 0x9fff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( sstar28k )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Sound")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Display Move")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Set Up")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Multi Move")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Color")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Replay")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Move")
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void star_state::sstar28k(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 2000000); // no XTAL
	m_maincpu->set_addrmap(AS_PROGRAM, &star_state::sstar28k_map);

	const attotime nmi_period = attotime::from_hz(2000000 / 0x2000); // 4020 Q13
	m_maincpu->set_periodic_int(FUNC(star_state::nmi_line_pulse), nmi_period);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(10, 3);
	config.set_default_layout(layout_saitek_sstar28k);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void star_state::tstar432(machine_config &config)
{
	sstar28k(config);

	/* basic machine hardware */
	R65C02(config.replace(), m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &star_state::tstar432_map);

	const attotime nmi_period = attotime::from_hz(4_MHz_XTAL / 0x4000); // 4020 Q14
	m_maincpu->set_periodic_int(FUNC(star_state::nmi_line_pulse), nmi_period);

	config.set_default_layout(layout_saitek_tstar432);

	/* extension rom */
	GENERIC_SOCKET(config, "extrom", generic_plain_slot, "saitek_kso");
	SOFTWARE_LIST(config, "cart_list").set_original("saitek_kso");
}

void star_state::sstar36k(machine_config &config)
{
	tstar432(config);

	/* basic machine hardware */
	M6502(config.replace(), m_maincpu, 2_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &star_state::tstar432_map);

	const attotime nmi_period = attotime::from_hz(2_MHz_XTAL / 0x2000); // 4020 Q13
	m_maincpu->set_periodic_int(FUNC(star_state::nmi_line_pulse), nmi_period);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( sstar28k )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("yo1c-v25_a0.u6", 0xa000, 0x2000, CRC(3c4bef09) SHA1(50349820d131db0138bd5dc4b62f38cc3aa1d7db) ) // M5L2764K
	ROM_LOAD("yo1c-v21_c0.u4", 0xc000, 0x2000, CRC(aae43b1b) SHA1(9acef9593f19ec3a6e9a671e82196d7bd054960e) ) // "
	ROM_LOAD("yo1c-v25_e0.u5", 0xe000, 0x2000, CRC(371b81fe) SHA1(c08dd0de8eebd7c1ed2d2281bf0241a83ee0f391) ) // "
ROM_END

ROM_START( tstar432 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("yo1d-j.u6", 0x8000, 0x8000, CRC(aa993096) SHA1(06db69a284eaf022b26e1087e09d8d459d270d03) )
ROM_END

ROM_START( tstar432a ) // only 1 byte difference (plus checksum), compared to sstar36k
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("tstar432a_80.bin", 0x8000, 0x4000, CRC(4f19f20c) SHA1(b921a440e8ab09a2a5e5f85d8aad3e4d7d815182) )
	ROM_LOAD("tstar432a_c0.bin", 0xc000, 0x2000, CRC(aae43b1b) SHA1(9acef9593f19ec3a6e9a671e82196d7bd054960e) )
	ROM_LOAD("tstar432a_e0.bin", 0xe000, 0x2000, CRC(6c504920) SHA1(588e23c9daff8a301c2f3d4f0c3fe62709f3accc) )
ROM_END

ROM_START( sstar36k )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("yo1d.u6", 0x8000, 0x8000, CRC(270c9a81) SHA1(5c9ef3a140651d7c9d9b801f2524cb93b0f92bb4) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME       PARENT  COMP MACHINE   INPUT     STATE       INIT        COMPANY, FULLNAME, FLAGS
CONS( 1983, sstar28k,  0,        0, sstar28k, sstar28k, star_state, empty_init, "SciSys", "Superstar 28K", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1985, tstar432,  0,        0, tstar432, sstar28k, star_state, empty_init, "SciSys", "Kasparov Turbostar 432 (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1985, tstar432a, tstar432, 0, tstar432, sstar28k, star_state, empty_init, "SciSys", "Kasparov Turbostar 432 (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1985, sstar36k,  tstar432, 0, sstar36k, sstar28k, star_state, empty_init, "SciSys", "Superstar 36K", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
