#############
#
# Fabric clock assignment
#
#############
# This clock need to route to fabric slot #0
# set_clock_pin -device_clock clk[0] -design_clock clk0 (Physical port name, clock module: CLK_BUF $clkbuf$top.$ibuf_clk0)
# set_clock_pin -device_clock clk[0] -design_clock $clk_buf_$ibuf_clk0 (Original clock primitive out-net to fabric)
set_clock_pin   -device_clock clk[0] -design_clock $clk_buf_$ibuf_clk0

# This clock is only used by gearbox, does not need to route to fabric slot #1
# set_clock_pin -device_clock clk[1] -design_clock clk1 (Physical port name, clock module: CLK_BUF clk_buf)

# This clock is only used by gearbox, does not need to route to fabric slot #2
# set_clock_pin -device_clock clk[2] -design_clock clk1 (Physical port name, clock module: PLL pll)

# This clock need to route to fabric slot #3
# set_clock_pin -device_clock clk[3] -design_clock clk2 (Physical port name, clock module: CLK_BUF $clkbuf$top.$ibuf_clk2)
# set_clock_pin -device_clock clk[3] -design_clock $clk_buf_$ibuf_clk2 (Original clock primitive out-net to fabric)
set_clock_pin   -device_clock clk[3] -design_clock $clk_buf_$ibuf_clk2

# This clock need to route to fabric slot #4
# set_clock_pin -device_clock clk[4] -design_clock din_serdes (Physical port name, clock module: I_SERDES i_serdes)
# set_clock_pin -device_clock clk[4] -design_clock iserdes_clk_out (Original clock primitive out-net to fabric)
set_clock_pin   -device_clock clk[4] -design_clock iserdes_clk_out

# This clock is only used by gearbox, does not need to route to fabric slot #5
# set_clock_pin -device_clock clk[5] -design_clock BOOT_CLOCK#0 (Physical port name, clock module: PLL pll_osc)

# This clock need to route to fabric slot #6
# set_clock_pin -device_clock clk[6] -design_clock dma_clk (Physical port name, clock module: CLK_BUF clk_buf_clk)
# set_clock_pin -device_clock clk[6] -design_clock clkbuf_dma_clk (Original clock primitive out-net to fabric)
set_clock_pin   -device_clock clk[6] -design_clock clkbuf_dma_clk

# This clock need to route to fabric slot #7
# This is fabric clock buffer
# set_clock_pin -device_clock clk[7] -design_clock FABRIC_CLKBUF#0 (Physical port name, clock module: FCLK_BUF $clkbuf$top.clk0_div)
# set_clock_pin -device_clock clk[7] -design_clock $fclk_buf_clk0_div (Original clock primitive out-net to fabric)
set_clock_pin   -device_clock clk[7] -design_clock $fclk_buf_clk0_div

# For fabric clock buffer output
# set_clock_out -device_clock clk[0] -design_clock clk0_div
set_clock_out   -device_clock clk[0] -design_clock clk0_div

#############
#
# Each pin mode and location assignment
#
#############
# Skip reason: Clock data from module I_BUF object clk0 port O does not need to route to fabric
# Pin      clk0                                  :: I_BUF |-> CLK_BUF

# Skip reason: Object clk1 is primitive \PLL but data signal is not defined
# Pin      clk1                                  :: I_BUF |-> CLK_BUF |-> PLL

# Skip reason: Clock data from module I_BUF object clk2 port O does not need to route to fabric
# Pin      clk2                                  :: I_BUF |-> CLK_BUF

# Pin      din                                   :: I_BUF |-> I_DELAY
# set_mode MODE_BP_DIR_A_RX                      HP_1_20_10P
# set_io   din                                   HP_1_20_10P                  --> (original)
set_io     din_delay                             HP_1_20_10P                  -mode          MODE_BP_DIR_A_RX -internal_pin g2f_rx_in[0]_A

# Pin      din_clk2                              :: I_BUF
# set_mode MODE_BP_DIR_A_RX                      HR_5_0_0P
# set_io   din_clk2                              HR_5_0_0P                    --> (original)
set_io     $ibuf_din_clk2                        HR_5_0_0P                    -mode          MODE_BP_DIR_A_RX -internal_pin g2f_rx_in[0]_A

# Pin      din_serdes                            :: I_BUF |-> I_SERDES
# set_mode MODE_RATE_8_A_RX                      HR_2_0_0P
# set_io   din_serdes                            HR_2_0_0P                    --> (original)
set_io     serdes_data[0]                        HR_2_0_0P                    -mode          MODE_RATE_8_A_RX -internal_pin g2f_rx_in[0]_A
set_io     serdes_data[1]                        HR_2_0_0P                    -mode          MODE_RATE_8_A_RX -internal_pin g2f_rx_in[1]_A
set_io     serdes_data[2]                        HR_2_0_0P                    -mode          MODE_RATE_8_A_RX -internal_pin g2f_rx_in[2]_A
set_io     serdes_data[3]                        HR_2_0_0P                    -mode          MODE_RATE_8_A_RX -internal_pin g2f_rx_in[3]_A
set_io     serdes_data[4]                        HR_2_0_0P                    -mode          MODE_RATE_8_A_RX -internal_pin g2f_rx_in[4]_A
set_io     serdes_data[5]                        HR_2_0_0P                    -mode          MODE_RATE_8_A_RX -internal_pin g2f_rx_in[5]_A
set_io     serdes_data[6]                        HR_2_0_0P                    -mode          MODE_RATE_8_A_RX -internal_pin g2f_rx_in[6]_A
set_io     serdes_data[7]                        HR_2_0_0P                    -mode          MODE_RATE_8_A_RX -internal_pin g2f_rx_in[7]_A

# Pin      din_serdes_clk_out                    :: I_BUF
# set_mode MODE_BP_DIR_A_RX                      HR_2_6_3P
# set_io   din_serdes_clk_out                    HR_2_6_3P                    --> (original)
set_io     $ibuf_din_serdes_clk_out              HR_2_6_3P                    -mode          MODE_BP_DIR_A_RX -internal_pin g2f_rx_in[0]_A

# Pin      dma_req[0]                            :: I_BUF
# set_mode MODE_BP_DIR_A_RX                      HR_5_20_10P
# set_io   dma_req[0]                            HR_5_20_10P                  --> (original)
set_io     $ibuf_dma_req[0]                      HR_5_20_10P                  -mode          MODE_BP_DIR_A_RX -internal_pin g2f_rx_in[0]_A

# Pin      dma_req[1]                            :: I_BUF
# set_mode MODE_BP_DIR_B_RX                      HR_5_21_10N
# set_io   dma_req[1]                            HR_5_21_10N                  --> (original)
set_io     $ibuf_dma_req[1]                      HR_5_20_10P                  -mode          MODE_BP_DIR_B_RX -internal_pin g2f_rx_in[5]_A

# Pin      dma_req[2]                            :: I_BUF
# set_mode MODE_BP_DIR_A_RX                      HR_5_22_11P
# set_io   dma_req[2]                            HR_5_22_11P                  --> (original)
set_io     $ibuf_dma_req[2]                      HR_5_22_11P                  -mode          MODE_BP_DIR_A_RX -internal_pin g2f_rx_in[0]_A

# Pin      dma_req[3]                            :: I_BUF
# set_mode MODE_BP_DIR_B_RX                      HR_5_23_11N
# set_io   dma_req[3]                            HR_5_23_11N                  --> (original)
set_io     $ibuf_dma_req[3]                      HR_5_22_11P                  -mode          MODE_BP_DIR_B_RX -internal_pin g2f_rx_in[5]_A

# Pin      dma_rst_n                             :: I_BUF
# set_mode MODE_BP_DIR_B_RX                      HR_5_29_14N
# set_io   dma_rst_n                             HR_5_29_14N                  --> (original)
set_io     $ibuf_dma_rst_n                       HR_5_28_14P                  -mode          MODE_BP_DIR_B_RX -internal_pin g2f_rx_in[5]_A

# Pin location is not assigned
# Pin      enable                                :: I_BUF

# Pin      reset                                 :: I_BUF
# set_mode MODE_BP_DIR_A_RX                      HP_1_0_0P
# set_io   reset                                 HP_1_0_0P                    --> (original)
set_io     $ibuf_reset                           HP_1_0_0P                    -mode          MODE_BP_DIR_A_RX -internal_pin g2f_rx_in[0]_A

# Skip reason: Object clk_out is primitive \O_SERDES_CLK but data signal is not defined
# Pin      clk_out                               :: O_SERDES_CLK |-> O_BUFT

# Pin      delay_tap[0]                          :: O_BUFT
# set_mode MODE_BP_DIR_A_TX                      HR_2_20_10P
# set_io   delay_tap[0]                          HR_2_20_10P                  --> (original)
set_io     $f2g_tx_out_$obuf_delay_tap[0]        HR_2_20_10P                  -mode          MODE_BP_DIR_A_TX -internal_pin f2g_tx_out[0]_A

# Pin      delay_tap[1]                          :: O_BUFT
# set_mode MODE_BP_DIR_A_TX                      HR_2_22_11P
# set_io   delay_tap[1]                          HR_2_22_11P                  --> (original)
set_io     $f2g_tx_out_$obuf_delay_tap[1]        HR_2_22_11P                  -mode          MODE_BP_DIR_A_TX -internal_pin f2g_tx_out[0]_A

# Pin      delay_tap[2]                          :: O_BUFT
# set_mode MODE_BP_DIR_A_TX                      HR_2_24_12P
# set_io   delay_tap[2]                          HR_2_24_12P                  --> (original)
set_io     $f2g_tx_out_$obuf_delay_tap[2]        HR_2_24_12P                  -mode          MODE_BP_DIR_A_TX -internal_pin f2g_tx_out[0]_A

# Pin      delay_tap[3]                          :: O_BUFT
# set_mode MODE_BP_DIR_A_TX                      HR_2_26_13P
# set_io   delay_tap[3]                          HR_2_26_13P                  --> (original)
set_io     $f2g_tx_out_$obuf_delay_tap[3]        HR_2_26_13P                  -mode          MODE_BP_DIR_A_TX -internal_pin f2g_tx_out[0]_A

# Pin      delay_tap[4]                          :: O_BUFT
# set_mode MODE_BP_DIR_A_TX                      HR_2_28_14P
# set_io   delay_tap[4]                          HR_2_28_14P                  --> (original)
set_io     $f2g_tx_out_$obuf_delay_tap[4]        HR_2_28_14P                  -mode          MODE_BP_DIR_A_TX -internal_pin f2g_tx_out[0]_A

# Pin      delay_tap[5]                          :: O_BUFT
# set_mode MODE_BP_DIR_A_TX                      HR_2_30_15P
# set_io   delay_tap[5]                          HR_2_30_15P                  --> (original)
set_io     $f2g_tx_out_$obuf_delay_tap[5]        HR_2_30_15P                  -mode          MODE_BP_DIR_A_TX -internal_pin f2g_tx_out[0]_A

# Pin      dma_ack[0]                            :: O_BUFT
# set_mode MODE_BP_DIR_A_TX                      HR_5_24_12P
# set_io   dma_ack[0]                            HR_5_24_12P                  --> (original)
set_io     $f2g_tx_out_$obuf_dma_ack[0]          HR_5_24_12P                  -mode          MODE_BP_DIR_A_TX -internal_pin f2g_tx_out[0]_A

# Pin      dma_ack[1]                            :: O_BUFT
# set_mode MODE_BP_DIR_B_TX                      HR_5_25_12N
# set_io   dma_ack[1]                            HR_5_25_12N                  --> (original)
set_io     $f2g_tx_out_$obuf_dma_ack[1]          HR_5_24_12P                  -mode          MODE_BP_DIR_B_TX -internal_pin f2g_tx_out[5]_A

# Pin      dma_ack[2]                            :: O_BUFT
# set_mode MODE_BP_DIR_A_TX                      HR_5_26_13P
# set_io   dma_ack[2]                            HR_5_26_13P                  --> (original)
set_io     $f2g_tx_out_$obuf_dma_ack[2]          HR_5_26_13P                  -mode          MODE_BP_DIR_A_TX -internal_pin f2g_tx_out[0]_A

# Pin      dma_ack[3]                            :: O_BUFT
# set_mode MODE_BP_DIR_B_TX                      HR_5_27_13N
# set_io   dma_ack[3]                            HR_5_27_13N                  --> (original)
set_io     $f2g_tx_out_$obuf_dma_ack[3]          HR_5_26_13P                  -mode          MODE_BP_DIR_B_TX -internal_pin f2g_tx_out[5]_A

# Pin      dout                                  :: O_DELAY |-> O_BUFT
# set_mode MODE_BP_DIR_A_TX                      HP_2_20_10P
# set_io   dout                                  HP_2_20_10P                  --> (original)
set_io     $f2g_tx_out_dout_pre_delay            HP_2_20_10P                  -mode          MODE_BP_DIR_A_TX -internal_pin f2g_tx_out[0]_A

# Pin      dout_clk2                             :: O_BUFT
# set_mode MODE_BP_DIR_B_TX                      HR_5_1_0N
# set_io   dout_clk2                             HR_5_1_0N                    --> (original)
set_io     $f2g_tx_out_$obuf_dout_clk2           HR_5_0_0P                    -mode          MODE_BP_DIR_B_TX -internal_pin f2g_tx_out[5]_A

# Pin      dout_serdes                           :: O_SERDES |-> O_BUFT
# set_mode MODE_RATE_8_A_TX                      HR_2_2_1P
# set_io   dout_serdes                           HR_2_2_1P                    --> (original)
set_io     $f2g_tx_out_serdes_data[0]            HR_2_2_1P                    -mode          MODE_RATE_8_A_TX -internal_pin f2g_tx_out[0]_A
set_io     $f2g_tx_out_serdes_data[1]            HR_2_2_1P                    -mode          MODE_RATE_8_A_TX -internal_pin f2g_tx_out[1]_A
set_io     $f2g_tx_out_serdes_data[2]            HR_2_2_1P                    -mode          MODE_RATE_8_A_TX -internal_pin f2g_tx_out[2]_A
set_io     $f2g_tx_out_serdes_data[3]            HR_2_2_1P                    -mode          MODE_RATE_8_A_TX -internal_pin f2g_tx_out[3]_A
set_io     $f2g_tx_out_serdes_data[4]            HR_2_2_1P                    -mode          MODE_RATE_8_A_TX -internal_pin f2g_tx_out[4]_A
set_io     $f2g_tx_out_serdes_data[5]            HR_2_2_1P                    -mode          MODE_RATE_8_A_TX -internal_pin f2g_tx_out[5]_A
set_io     $f2g_tx_out_serdes_data[6]            HR_2_2_1P                    -mode          MODE_RATE_8_A_TX -internal_pin f2g_tx_out[6]_A
set_io     $f2g_tx_out_serdes_data[7]            HR_2_2_1P                    -mode          MODE_RATE_8_A_TX -internal_pin f2g_tx_out[7]_A

# Pin      dout_serdes_clk_out                   :: O_BUFT
# set_mode MODE_BP_DIR_B_TX                      HR_2_7_3N
# set_io   dout_serdes_clk_out                   HR_2_7_3N                    --> (original)
set_io     $f2g_tx_out_$obuf_dout_serdes_clk_out HR_2_6_3P                    -mode          MODE_BP_DIR_B_TX -internal_pin f2g_tx_out[5]_A

# Skip reason: Clock data from module I_BUF object dma_clk port O does not need to route to fabric
# Pin      dma_clk                               :: I_BUF |-> CLK_BUF

# Skip this because 'This is secondary pin. But IO bitstream generation will still make sure it is used in pair. Otherwise the IO bitstream will be invalid'
# Pin      din_n                                 :: I_BUF_DS |-> I_DDR

# Pin      din_p                                 :: I_BUF_DS |-> I_DDR
# set_mode MODE_BP_DDR_A_RX                      HP_1_4_2P
# set_io   din_p                                 HP_1_4_2P                    --> (original)
set_io     o_ddr_d[0]                            HP_1_4_2P                    -mode          MODE_BP_DDR_A_RX -internal_pin g2f_rx_in[0]_A
set_io     o_ddr_d[1]                            HP_1_4_2P                    -mode          MODE_BP_DDR_A_RX -internal_pin g2f_rx_in[1]_A

# Skip this because 'This is secondary pin. But IO bitstream generation will still make sure it is used in pair. Otherwise the IO bitstream will be invalid'
# Pin      dout_n                                :: O_DDR |-> O_BUFT_DS

# Pin      dout_p                                :: O_DDR |-> O_BUFT_DS
# set_mode MODE_BP_DDR_A_TX                      HP_1_8_4P
# set_io   dout_p                                HP_1_8_4P                    --> (original)
set_io     $f2g_tx_out_o_ddr_d[0]                HP_1_8_4P                    -mode          MODE_BP_DDR_A_TX -internal_pin f2g_tx_out[0]_A
set_io     $f2g_tx_out_o_ddr_d[1]                HP_1_8_4P                    -mode          MODE_BP_DDR_A_TX -internal_pin f2g_tx_out[1]_A

# Skip this because 'This is secondary pin. But IO bitstream generation will still make sure it is used in pair. Otherwise the IO bitstream will be invalid'
# Pin      dout_osc_n                            :: O_DDR |-> O_BUFT_DS

# Pin      dout_osc_p                            :: O_DDR |-> O_BUFT_DS
# set_mode MODE_BP_DDR_A_TX                      HP_2_22_11P
# set_io   dout_osc_p                            HP_2_22_11P                  --> (original)
set_io     $f2g_tx_out_o_ddr_d[0]_2              HP_2_22_11P                  -mode          MODE_BP_DDR_A_TX -internal_pin f2g_tx_out[0]_A
set_io     $f2g_tx_out_o_ddr_d[1]_2              HP_2_22_11P                  -mode          MODE_BP_DDR_A_TX -internal_pin f2g_tx_out[1]_A

#############
#
# Internal Control Signals
#
#############
# Module: I_BUF
# LinkedObject: clk0
# Location: HR_1_CC_18_9P
# Port: EN
# Signal: in:f2g_in_en_A
set_io   $auto_513                      HR_1_CC_18_9P -mode MODE_BP_DIR_A_RX -internal_pin f2g_in_en_A

# Module: I_BUF
# LinkedObject: clk1
# Location: HP_1_CC_18_9P
# Port: EN
# Signal: in:f2g_in_en_A
set_io   $auto_514                      HP_1_CC_18_9P -mode MODE_BP_DIR_A_RX -internal_pin f2g_in_en_A

# Module: I_BUF
# LinkedObject: clk2
# Location: HR_5_CC_18_9P
# Port: EN
# Signal: in:f2g_in_en_A
set_io   $auto_515                      HR_5_CC_18_9P -mode MODE_BP_DIR_A_RX -internal_pin f2g_in_en_A

# Module: I_BUF
# LinkedObject: din
# Location: HP_1_20_10P
# Port: EN
# Signal: in:f2g_in_en_A
set_io   $auto_516                      HP_1_20_10P   -mode MODE_BP_DIR_A_RX -internal_pin f2g_in_en_A

# Module: I_DELAY
# LinkedObject: din
# Location: HP_1_20_10P
# Port: DLY_ADJ
# Signal: in:f2g_trx_dly_adj
set_io   $auto_543                      HP_1_20_10P   -mode MODE_BP_DIR_A_RX -internal_pin f2g_trx_dly_adj

# Module: I_DELAY
# LinkedObject: din
# Location: HP_1_20_10P
# Port: DLY_INCDEC
# Signal: in:f2g_trx_dly_inc
set_io   $auto_544                      HP_1_20_10P   -mode MODE_BP_DIR_A_RX -internal_pin f2g_trx_dly_inc

# Module: I_DELAY
# LinkedObject: din
# Location: HP_1_20_10P
# Port: DLY_LOAD
# Signal: in:f2g_trx_dly_ld
set_io   $auto_545                      HP_1_20_10P   -mode MODE_BP_DIR_A_RX -internal_pin f2g_trx_dly_ld

# Module: I_DELAY
# LinkedObject: din
# Location: HP_1_20_10P
# Port: DLY_TAP_VALUE
# Signal: out:g2f_trx_dly_tap
set_io   $ifab_$obuf_delay_tap[0]       HP_1_20_10P   -mode MODE_BP_DIR_A_RX -internal_pin g2f_trx_dly_tap[0]
set_io   $ifab_$obuf_delay_tap[1]       HP_1_20_10P   -mode MODE_BP_DIR_A_RX -internal_pin g2f_trx_dly_tap[1]
set_io   $ifab_$obuf_delay_tap[2]       HP_1_20_10P   -mode MODE_BP_DIR_A_RX -internal_pin g2f_trx_dly_tap[2]
set_io   $ifab_$obuf_delay_tap[3]       HP_1_20_10P   -mode MODE_BP_DIR_A_RX -internal_pin g2f_trx_dly_tap[3]
set_io   $ifab_$obuf_delay_tap[4]       HP_1_20_10P   -mode MODE_BP_DIR_A_RX -internal_pin g2f_trx_dly_tap[4]
set_io   $ifab_$obuf_delay_tap[5]       HP_1_20_10P   -mode MODE_BP_DIR_A_RX -internal_pin g2f_trx_dly_tap[5]

# Module: I_BUF
# LinkedObject: din_clk2
# Location: HR_5_0_0P
# Port: EN
# Signal: in:f2g_in_en_A
set_io   $auto_517                      HR_5_0_0P     -mode MODE_BP_DIR_A_RX -internal_pin f2g_in_en_A

# Module: I_BUF
# LinkedObject: din_serdes
# Location: HR_2_0_0P
# Port: EN
# Signal: in:f2g_in_en_A
# Skip: Accpetable-conflict with primitive i_serdes port EN
# set_io $auto_518                      HR_2_0_0P     -mode MODE_RATE_8_A_RX -internal_pin f2g_in_en_A[0]

# Module: I_SERDES
# LinkedObject: din_serdes
# Location: HR_2_0_0P
# Port: BITSLIP_ADJ
# Signal: in:f2g_rx_bitslip_adj
set_io   $auto_546                      HR_2_0_0P     -mode MODE_RATE_8_A_RX -internal_pin f2g_rx_bitslip_adj

# Module: I_SERDES
# LinkedObject: din_serdes
# Location: HR_2_0_0P
# Port: DATA_VALID
# Signal: out:g2f_rx_dvalid_A
# Skip: User design does not utilize linked-object din_serdes wrapped-instance port DATA_VALID

# Module: I_SERDES
# LinkedObject: din_serdes
# Location: HR_2_0_0P
# Port: DPA_ERROR
# Signal: out:g2f_rx_dpa_error
# Skip: User design does not utilize linked-object din_serdes wrapped-instance port DPA_ERROR

# Module: I_SERDES
# LinkedObject: din_serdes
# Location: HR_2_0_0P
# Port: DPA_LOCK
# Signal: out:g2f_rx_dpa_lock
# Skip: User design does not utilize linked-object din_serdes wrapped-instance port DPA_LOCK

# Module: I_SERDES
# LinkedObject: din_serdes
# Location: HR_2_0_0P
# Port: EN
# Signal: in:f2g_in_en_A
set_io   $auto_547                      HR_2_0_0P     -mode MODE_RATE_8_A_RX -internal_pin f2g_in_en_A

# Module: I_SERDES
# LinkedObject: din_serdes
# Location: HR_2_0_0P
# Port: RST
# Signal: in:f2g_trx_reset_n_A
set_io   $auto_548                      HR_2_0_0P     -mode MODE_RATE_8_A_RX -internal_pin f2g_trx_reset_n_A

# Module: I_BUF
# LinkedObject: din_serdes_clk_out
# Location: HR_2_6_3P
# Port: EN
# Signal: in:f2g_in_en_A
set_io   $auto_519                      HR_2_6_3P     -mode MODE_BP_DIR_A_RX -internal_pin f2g_in_en_A

# Module: I_BUF
# LinkedObject: dma_req[0]
# Location: HR_5_20_10P
# Port: EN
# Signal: in:f2g_in_en_A
set_io   $auto_520                      HR_5_20_10P   -mode MODE_BP_DIR_A_RX -internal_pin f2g_in_en_A

# Module: I_BUF
# LinkedObject: dma_req[1]
# Location: HR_5_21_10N
# Port: EN
# Signal: in:f2g_in_en_B
set_io   $auto_521                      HR_5_21_10N   -mode MODE_BP_DIR_B_RX -internal_pin f2g_in_en_B

# Module: I_BUF
# LinkedObject: dma_req[2]
# Location: HR_5_22_11P
# Port: EN
# Signal: in:f2g_in_en_A
set_io   $auto_522                      HR_5_22_11P   -mode MODE_BP_DIR_A_RX -internal_pin f2g_in_en_A

# Module: I_BUF
# LinkedObject: dma_req[3]
# Location: HR_5_23_11N
# Port: EN
# Signal: in:f2g_in_en_B
set_io   $auto_523                      HR_5_23_11N   -mode MODE_BP_DIR_B_RX -internal_pin f2g_in_en_B

# Module: I_BUF
# LinkedObject: dma_rst_n
# Location: HR_5_29_14N
# Port: EN
# Signal: in:f2g_in_en_B
set_io   $auto_524                      HR_5_29_14N   -mode MODE_BP_DIR_B_RX -internal_pin f2g_in_en_B

# Module: I_BUF
# LinkedObject: enable
# Location: 
# Port: EN
# Signal: in:f2g_in_en_{A|B}
# Skip: Location is not assigned

# Module: I_BUF
# LinkedObject: reset
# Location: HP_1_0_0P
# Port: EN
# Signal: in:f2g_in_en_A
set_io   $auto_526                      HP_1_0_0P     -mode MODE_BP_DIR_A_RX -internal_pin f2g_in_en_A

# Module: O_BUFT
# LinkedObject: clk_out
# Location: HR_2_4_2P
# Port: T
# Signal: in:f2g_tx_oe_A
set_io   $auto_527                      HR_2_4_2P     -mode MODE_BP_SDR_A_TX -internal_pin f2g_tx_oe_A

# Module: O_SERDES_CLK
# LinkedObject: clk_out
# Location: HR_2_4_2P
# Port: CLK_EN
# Signal: in:f2g_tx_clk_en_A
set_io   $auto_557                      HR_2_4_2P     -mode MODE_BP_SDR_A_TX -internal_pin f2g_tx_clk_en_A

# Module: O_BUFT
# LinkedObject: delay_tap[0]
# Location: HR_2_20_10P
# Port: T
# Signal: in:f2g_tx_oe_A
set_io   $auto_528                      HR_2_20_10P   -mode MODE_BP_DIR_A_TX -internal_pin f2g_tx_oe_A

# Module: O_BUFT
# LinkedObject: delay_tap[1]
# Location: HR_2_22_11P
# Port: T
# Signal: in:f2g_tx_oe_A
set_io   $auto_529                      HR_2_22_11P   -mode MODE_BP_DIR_A_TX -internal_pin f2g_tx_oe_A

# Module: O_BUFT
# LinkedObject: delay_tap[2]
# Location: HR_2_24_12P
# Port: T
# Signal: in:f2g_tx_oe_A
set_io   $auto_530                      HR_2_24_12P   -mode MODE_BP_DIR_A_TX -internal_pin f2g_tx_oe_A

# Module: O_BUFT
# LinkedObject: delay_tap[3]
# Location: HR_2_26_13P
# Port: T
# Signal: in:f2g_tx_oe_A
set_io   $auto_531                      HR_2_26_13P   -mode MODE_BP_DIR_A_TX -internal_pin f2g_tx_oe_A

# Module: O_BUFT
# LinkedObject: delay_tap[4]
# Location: HR_2_28_14P
# Port: T
# Signal: in:f2g_tx_oe_A
set_io   $auto_532                      HR_2_28_14P   -mode MODE_BP_DIR_A_TX -internal_pin f2g_tx_oe_A

# Module: O_BUFT
# LinkedObject: delay_tap[5]
# Location: HR_2_30_15P
# Port: T
# Signal: in:f2g_tx_oe_A
set_io   $auto_533                      HR_2_30_15P   -mode MODE_BP_DIR_A_TX -internal_pin f2g_tx_oe_A

# Module: O_BUFT
# LinkedObject: dma_ack[0]
# Location: HR_5_24_12P
# Port: T
# Signal: in:f2g_tx_oe_A
set_io   $auto_534                      HR_5_24_12P   -mode MODE_BP_DIR_A_TX -internal_pin f2g_tx_oe_A

# Module: O_BUFT
# LinkedObject: dma_ack[1]
# Location: HR_5_25_12N
# Port: T
# Signal: in:f2g_tx_oe_B
set_io   $auto_535                      HR_5_25_12N   -mode MODE_BP_DIR_B_TX -internal_pin f2g_tx_oe_B

# Module: O_BUFT
# LinkedObject: dma_ack[2]
# Location: HR_5_26_13P
# Port: T
# Signal: in:f2g_tx_oe_A
set_io   $auto_536                      HR_5_26_13P   -mode MODE_BP_DIR_A_TX -internal_pin f2g_tx_oe_A

# Module: O_BUFT
# LinkedObject: dma_ack[3]
# Location: HR_5_27_13N
# Port: T
# Signal: in:f2g_tx_oe_B
set_io   $auto_537                      HR_5_27_13N   -mode MODE_BP_DIR_B_TX -internal_pin f2g_tx_oe_B

# Module: O_BUFT
# LinkedObject: dout
# Location: HP_2_20_10P
# Port: T
# Signal: in:f2g_tx_oe_A
set_io   $auto_538                      HP_2_20_10P   -mode MODE_BP_DIR_A_TX -internal_pin f2g_tx_oe_A

# Module: O_DELAY
# LinkedObject: dout
# Location: HP_2_20_10P
# Port: DLY_ADJ
# Signal: in:f2g_trx_dly_adj
set_io   $auto_551                      HP_2_20_10P   -mode MODE_BP_DIR_A_TX -internal_pin f2g_trx_dly_adj

# Module: O_DELAY
# LinkedObject: dout
# Location: HP_2_20_10P
# Port: DLY_INCDEC
# Signal: in:f2g_trx_dly_inc
set_io   $auto_552                      HP_2_20_10P   -mode MODE_BP_DIR_A_TX -internal_pin f2g_trx_dly_inc

# Module: O_DELAY
# LinkedObject: dout
# Location: HP_2_20_10P
# Port: DLY_LOAD
# Signal: in:f2g_trx_dly_ld
set_io   $auto_553                      HP_2_20_10P   -mode MODE_BP_DIR_A_TX -internal_pin f2g_trx_dly_ld

# Module: O_DELAY
# LinkedObject: dout
# Location: HP_2_20_10P
# Port: DLY_TAP_VALUE
# Signal: out:g2f_trx_dly_tap
# Skip: User design does not utilize linked-object dout wrapped-instance port DLY_TAP_VALUE

# Module: O_BUFT
# LinkedObject: dout_clk2
# Location: HR_5_1_0N
# Port: T
# Signal: in:f2g_tx_oe_B
set_io   $auto_539                      HR_5_1_0N     -mode MODE_BP_DIR_B_TX -internal_pin f2g_tx_oe_B

# Module: O_BUFT
# LinkedObject: dout_serdes
# Location: HR_2_2_1P
# Port: T
# Signal: in:f2g_tx_oe_A
set_io   $auto_540                      HR_2_2_1P     -mode MODE_RATE_8_A_TX -internal_pin f2g_tx_oe_A

# Module: O_SERDES
# LinkedObject: dout_serdes
# Location: HR_2_2_1P
# Port: DATA_VALID
# Signal: in:f2g_tx_dvalid_A
set_io   $auto_554                      HR_2_2_1P     -mode MODE_RATE_8_A_TX -internal_pin f2g_tx_dvalid_A

# Module: O_SERDES
# LinkedObject: dout_serdes
# Location: HR_2_2_1P
# Port: OE_IN
# Signal: in:f2g_in_en_A
set_io   $auto_555                      HR_2_2_1P     -mode MODE_RATE_8_A_TX -internal_pin f2g_in_en_A

# Module: O_SERDES
# LinkedObject: dout_serdes
# Location: HR_2_2_1P
# Port: RST
# Signal: in:f2g_trx_reset_n_A
set_io   $auto_556                      HR_2_2_1P     -mode MODE_RATE_8_A_TX -internal_pin f2g_trx_reset_n_A

# Module: O_BUFT
# LinkedObject: dout_serdes_clk_out
# Location: HR_2_7_3N
# Port: T
# Signal: in:f2g_tx_oe_B
set_io   $auto_541                      HR_2_7_3N     -mode MODE_BP_DIR_B_TX -internal_pin f2g_tx_oe_B

# Module: I_BUF
# LinkedObject: dma_clk
# Location: HR_5_28_14P
# Port: EN
# Signal: in:f2g_in_en_A
# Skip: User design does not utilize linked-object dma_clk wrapped-instance port EN

# Module: I_BUF_DS
# LinkedObject: din_n+din_p
# Location: HP_1_4_2P
# Port: EN
# Signal: in:f2g_in_en_A
# Skip: Accpetable-conflict with primitive i_ddr port E
# set_io $auto_542                      HP_1_4_2P     -mode MODE_BP_DDR_A_RX -internal_pin f2g_in_en_A[0]

# Module: I_DDR
# LinkedObject: din_n+din_p
# Location: HP_1_4_2P
# Port: E
# Signal: in:f2g_in_en_A
set_io   $ofab_$ibuf_enable_4           HP_1_4_2P     -mode MODE_BP_DDR_A_RX -internal_pin f2g_in_en_A

# Module: I_DDR
# LinkedObject: din_n+din_p
# Location: HP_1_4_2P
# Port: R
# Signal: in:f2g_trx_reset_n_A
set_io   $f2g_trx_reset_n_$ibuf_reset_4 HP_1_4_2P     -mode MODE_BP_DDR_A_RX -internal_pin f2g_trx_reset_n_A

# Module: O_BUFT_DS
# LinkedObject: dout_n+dout_p
# Location: HP_1_8_4P
# Port: T
# Signal: in:f2g_tx_oe_A
# Skip: Accpetable-conflict with primitive o_ddr port E
# set_io $auto_549                      HP_1_8_4P     -mode MODE_BP_DDR_A_TX -internal_pin f2g_tx_oe_A[0]

# Module: O_DDR
# LinkedObject: dout_n+dout_p
# Location: HP_1_8_4P
# Port: E
# Signal: in:f2g_tx_oe_A
set_io   $ofab_$ibuf_enable             HP_1_8_4P     -mode MODE_BP_DDR_A_TX -internal_pin f2g_tx_oe_A

# Module: O_DDR
# LinkedObject: dout_n+dout_p
# Location: HP_1_8_4P
# Port: R
# Signal: in:f2g_trx_reset_n_A
set_io   $f2g_trx_reset_n_$ibuf_reset   HP_1_8_4P     -mode MODE_BP_DDR_A_TX -internal_pin f2g_trx_reset_n_A

# Module: O_BUFT_DS
# LinkedObject: dout_osc_n+dout_osc_p
# Location: HP_2_22_11P
# Port: T
# Signal: in:f2g_tx_oe_A
# Skip: Accpetable-conflict with primitive o_ddr_osc port E
# set_io $auto_550                      HP_2_22_11P   -mode MODE_BP_DDR_A_TX -internal_pin f2g_tx_oe_A[0]

# Module: O_DDR
# LinkedObject: dout_osc_n+dout_osc_p
# Location: HP_2_22_11P
# Port: E
# Signal: in:f2g_tx_oe_A
set_io   $ofab_$ibuf_enable_2           HP_2_22_11P   -mode MODE_BP_DDR_A_TX -internal_pin f2g_tx_oe_A

# Module: O_DDR
# LinkedObject: dout_osc_n+dout_osc_p
# Location: HP_2_22_11P
# Port: R
# Signal: in:f2g_trx_reset_n_A
set_io   $f2g_trx_reset_n_$ibuf_reset_2 HP_2_22_11P   -mode MODE_BP_DDR_A_TX -internal_pin f2g_trx_reset_n_A

#############
#
# Each gearbox core clock
#
#############
# Module: I_DELAY
# Name: i_delay
# Location: HP_1_20_10P
# Port: CLK_IN
# Net: $clk_buf_$ibuf_clk0
# Slot: 0
set_core_clk HP_1_20_10P 0

# Module: I_SERDES
# Name: i_serdes
# Location: HR_2_0_0P
# Port: CLK_IN
# Net: pll_clk
# Slot: 2
set_core_clk HR_2_0_0P   2

# Module: O_DELAY
# Name: o_delay
# Location: HP_2_20_10P
# Port: CLK_IN
# Net: clk1_buf
# Slot: 1
set_core_clk HP_2_20_10P 1

# Module: O_SERDES
# Name: o_serdes
# Location: HR_2_2_1P
# Port: CLK_IN
# Net: pll_clk
# Slot: 2
set_core_clk HR_2_2_1P   2

# Module: O_DDR
# Name: o_ddr
# Location: HP_1_8_4P
# Port: C
# Net: pll_clk
# Slot: 2
set_core_clk HP_1_8_4P   2

# Module: O_DDR
# Name: o_ddr_osc
# Location: HP_2_22_11P
# Port: C
# Net: osc_pll
# Slot: 5
set_core_clk HP_2_22_11P 5


#############
#
# SOC Module: SOC_FPGA_INTF_DMA ($auto_569.dma)
#
#############
# Port: DMA_REQ
set_io $auto_561        VCC_HP_AUX -mode Mode_GPIO -internal_pin fpga_clk_dma_req[0]
set_io $auto_562        VCC_HP_AUX -mode Mode_GPIO -internal_pin fpga_clk_dma_req[1]
set_io $auto_563        VCC_HP_AUX -mode Mode_GPIO -internal_pin fpga_clk_dma_req[2]
set_io $auto_564        VCC_HP_AUX -mode Mode_GPIO -internal_pin fpga_clk_dma_req[3]

# Port: DMA_ACK
set_io $obuf_dma_ack[0] VCC_HP_AUX -mode Mode_GPIO -internal_pin fpga_clk_dma_ack[0]
set_io $obuf_dma_ack[1] VCC_HP_AUX -mode Mode_GPIO -internal_pin fpga_clk_dma_ack[1]
set_io $obuf_dma_ack[2] VCC_HP_AUX -mode Mode_GPIO -internal_pin fpga_clk_dma_ack[2]
set_io $obuf_dma_ack[3] VCC_HP_AUX -mode Mode_GPIO -internal_pin fpga_clk_dma_ack[3]

# Port: DMA_RST_N
set_io $auto_565        VCC_HP_AUX -mode Mode_GPIO -internal_pin rst_n_fpga_fabric_dma[0]

#############
#
# SOC clock assignment
#
#############
# Module: SOC_FPGA_INTF_DMA, Name: dma, Port: DMA_CLK, Net: \clkbuf_dma_clk, Mapping: clk_fpga_fabric_dma
set_soc_clk clk_fpga_fabric_dma 6

