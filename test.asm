        .AREA prg (abs)
        .org 0x0200
        sta  0x10
        lda 0x400
        ldx #0xff
        txs
L123:   dex
        sta  0x80,X
        beq  L123
        
