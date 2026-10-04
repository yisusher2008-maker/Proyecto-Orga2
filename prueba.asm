# Programa de prueba completo para OECISEY-1
inicio:
    IRMOVQ $10, %r1
    IRMOVQ $20, %r2
    ADDQ %r1, %r2
    SUBQ %r1, %r2
    ANDQ %r1, %r2
    XORQ %r1, %r2
    RRMVQ %r1, %r3
    PUSH %r1
    POP %r4
    IRMOVQ $0x05E36B05, %r5
    STD %r2, 0(%r5)
    STD %r3, 8(%r5)
    LDD %r6, 0(%r5)
    LDD %r7, 8(%r5)
    MRMOVQ %r1, 16(%r5)
    RMMOVQ %r2, 24(%r5)
    IRMOVQ $0, %r1
    IRMOVQ $5, %r2
    ADDQ %r1, %r2
    JNZ salto1
    IRMOVQ $999, %r3
salto1:
    IRMOVQ $1, %r1
    IRMOVQ $1, %r2
    SUBQ %r1, %r2
    JE salto2
    IRMOVQ $888, %r3
salto2:
    JMP fin
    IRMOVQ $777, %r3
fin:
    HALT