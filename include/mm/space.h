#ifndef SPACE_H_
#define SPACE_H_

#define VM_MAX_SEGMENTS 10

/*
 * Describe un segmento con su direccion de comienzo y 
 * su tamano.
 */
typedef struct 
{
	paddr_t seg_start;
	paddr_t seg_end;
	uint32_t seg_type;
} region_t;

/*
 * Describe un espacio de direcciones
 * con sus respectivos segmentos.
 * 
 * Los segmentos pueden no ser contiguos
 * por que pueden haber huecos entre ellos
 * que no se pueden direccionar.
 */
typedef struct
{
	region_t segments[VM_MAX_SEGMENTS];
	uint32_t segments_number;
} space_t;

#endif /*SPACE_H_*/
