/*=========================================================
    formatter.h
=========================================================*/

#ifndef FORMATTER_H
#define FORMATTER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_CHARGES            10
#define MAX_NARRATIVES         15
#define MAX_MAPPINGS           50

#define SEGMENT_TYPE_LEN       2
#define RESERVED_LEN           20
#define NARRATIVE_BLOCK_LEN    40
#define NARRATIVE_TEXT_LEN     30

#define OUTPUT_SEGMENT_TYPE    "BP"
#define OUTPUT_DELIMITER       "~~"

typedef enum
{
    SECTION_NONE,
    SECTION_CHARGES,
    SECTION_NARRATIVES
} section_type;

typedef struct
{
    char narrative_type[5];
    char destination;
    char narrative_data[256];

} gloss_narrative;

typedef struct
{
    char charge_type[5];
    char direction;
    long long amount;     /* implied 6 decimals */

} gloss_charge;

typedef struct
{
    char gloss_type[5];
    char intact_code[4];

} mapping_record;

typedef struct
{
    char narrative_code[4];
    char destination[3];
    char narrative_text[31];

} intact_narrative;

typedef struct
{
    char segment_type[3];
    int narrative_count;

    intact_narrative narratives[MAX_NARRATIVES];

    char delimiter[3];

} bps_intact_description;


/* prototypes */

void trim(char *str);

void fatal_error(const char *message);

void load_gloss_file(
    const char *filename,
    gloss_narrative narratives[],
    int *narrative_count,
    gloss_charge charges[],
    int *charge_count
);

void load_mappings(
    const char *filename,
    mapping_record mappings[],
    int *mapping_count
);

void parse_charge_line(
    char *line,
    gloss_charge *charge
);

void parse_narrative_line(
    char *line,
    gloss_narrative *narrative
);

long long parse_amount(const char *amount_str);

void validate_data(
    int narrative_count,
    int charge_count
);

mapping_record* find_mapping(
    mapping_record mappings[],
    int mapping_count,
    const char *gloss_type
);

void map_to_intact(
    gloss_narrative narratives[],
    int narrative_count,
    gloss_charge charges[],
    int charge_count,
    mapping_record mappings[],
    int mapping_count,
    bps_intact_description *intact
);

void sort_intact_narratives(
    intact_narrative narratives[],
    int count
);

int compare_narratives(
    const void *a,
    const void *b
);

void build_output_segment(
    bps_intact_description *intact,
    char *output
);

void write_output_file(
    const char *filename,
    const char *output
);

#endif