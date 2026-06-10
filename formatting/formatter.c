/*=========================================================
    formatter.c
=========================================================*/

#include "formatter.h"

/*=========================================================
    MAIN
=========================================================*/

int main(void)
{
    gloss_narrative narratives[MAX_NARRATIVES];
    gloss_charge charges[MAX_CHARGES];

    mapping_record mappings[MAX_MAPPINGS];

    bps_intact_description intact;

    char output_buffer[700];

    int narrative_count = 0;
    int charge_count = 0;
    int mapping_count = 0;


    load_gloss_file(
        "files/gloss_input.txt",
        narratives,
        &narrative_count,
        charges,
        &charge_count
    );

    validate_data(narrative_count, charge_count);

    load_mappings(
        "files/mappings.txt",
        mappings,
        &mapping_count
    );

    printf(
    "\nBefore mapping:\n"
    "Narratives = %d\n"
    "Charges    = %d\n",
    narrative_count,
    charge_count);

    map_to_intact(
        narratives,
        narrative_count,
        charges,
        charge_count,
        mappings,
        mapping_count,
        &intact
    );


    sort_intact_narratives(
        intact.narratives,
        intact.narrative_count
    );

    build_output_segment(
        &intact,
        output_buffer
    );

    write_output_file(
        "files/intact_output.txt",
        output_buffer
    );

    printf("Processing complete\n");

    return 0;
}


/*=========================================================
    UTILITY
=========================================================*/

void trim(char *str)
{
    char *start;
    char *end;

    start = str;

    while (isspace((unsigned char)*start))
    {
        start++;
    }

    if (start != str)
    {
        memmove(str, start, strlen(start) + 1);
    }

    end = str + strlen(str) - 1;

    while (end >= str &&
           isspace((unsigned char)*end))
    {
        *end = '\0';
        end--;
    }
}

void fatal_error(const char *message)
{
    fprintf(stderr, "ERROR: %s\n", message);
    exit(EXIT_FAILURE);
}


/*=========================================================
    LOAD GLOSS FILE
=========================================================*/

void load_gloss_file(
    const char *filename,
    gloss_narrative narratives[],
    int *narrative_count,
    gloss_charge charges[],
    int *charge_count
)
{
    FILE *fp;

    char line[512];

    section_type section = SECTION_NONE;

    fp = fopen(filename, "r");

    if (fp == NULL)
    {
        fatal_error("Unable to open gloss input file");
    }

    while (fgets(line, sizeof(line), fp))
    {
        trim(line);

        if (strcmp(line, "CHARGES") == 0)
        {
            section = SECTION_CHARGES;
            continue;
        }

        if (strcmp(line, "NARRATIVES") == 0)
        {
            section = SECTION_NARRATIVES;
            continue;
        }

        if (strcmp(line, "END") == 0)
        {
            section = SECTION_NONE;
            continue;
        }

        switch (section)
        {
            case SECTION_CHARGES:

                if (*charge_count >= MAX_CHARGES)
                {
                    fatal_error("Charge count exceeds maximum of 10");
                }

                parse_charge_line(
                    line,
                    &charges[*charge_count]
                );

                (*charge_count)++;

                break;

            case SECTION_NARRATIVES:

                if (*narrative_count >= MAX_NARRATIVES)
                {
                    fatal_error("Narrative count exceeds maximum of 15");
                }

                parse_narrative_line(
                    line,
                    &narratives[*narrative_count]
                );

                (*narrative_count)++;

                break;

            default:
                break;
        }
    }

    fclose(fp);
}


/*=========================================================
    PARSE CHARGE
=========================================================*/

void parse_charge_line(
    char *line,
    gloss_charge *charge
)
{
    char *token;

    token = strtok(line, ",");

    if (token == NULL)
    {
        fatal_error("Invalid charge type");
    }

    strncpy(charge->charge_type, token, 4);
    charge->charge_type[4] = '\0';


    token = strtok(NULL, ",");

    if (token == NULL)
    {
        fatal_error("Invalid charge direction");
    }

    charge->direction = token[0];


    token = strtok(NULL, ",");

    if (token == NULL)
    {
        fatal_error("Invalid charge amount");
    }

    charge->amount = parse_amount(token);
}


/*=========================================================
    PARSE NARRATIVE
=========================================================*/

void parse_narrative_line(
    char *line,
    gloss_narrative *narrative
)
{
    char *token;

    token = strtok(line, ",");

    if (token == NULL)
    {
        fatal_error("Invalid narrative type");
    }

    strncpy(narrative->narrative_type, token, 4);
    narrative->narrative_type[4] = '\0';


    token = strtok(NULL, ",");

    if (token == NULL)
    {
        fatal_error("Missing narrative destination");
    }

    narrative->destination = token[0];


    token = strtok(NULL, "");

    if (token == NULL)
    {
        fatal_error("Missing narrative text");
    }

    strncpy(narrative->narrative_data, token, 255);
    narrative->narrative_data[255] = '\0';
}


/*=========================================================
    PARSE AMOUNT
=========================================================*/

long long parse_amount(const char *amount_str)
{
    char buffer[32];

    int left_digits = 0;
    int right_digits = 0;

    int decimal_found = 0;

    int i;
    int j = 0;

    long long amount;

    for (i = 0; amount_str[i] != '\0'; i++)
    {
        if (amount_str[i] == '.')
        {
            decimal_found = 1;
            continue;
        }

        if (!isdigit((unsigned char)amount_str[i]))
        {
            fatal_error("Invalid amount format");
        }

        if (decimal_found)
        {
            right_digits++;
        }
        else
        {
            left_digits++;
        }

        if (j >= 15)
        {
            fatal_error("Amount exceeds maximum length");
        }

        buffer[j++] = amount_str[i];
    }

    buffer[j] = '\0';

    if (left_digits > 9)
    {
        fatal_error("Amount exceeds 9 digits left of decimal");
    }

    if (right_digits > 6)
    {
        fatal_error("Amount exceeds 6 decimals");
    }

    while (right_digits < 6)
    {
        buffer[j++] = '0';
        right_digits++;
    }

    buffer[j] = '\0';

    amount = atoll(buffer);

    return amount;
}


/*=========================================================
    VALIDATE
=========================================================*/

void validate_data(
    int narrative_count,
    int charge_count
)
{
    int total;

    total = narrative_count + charge_count;

    if (charge_count > MAX_CHARGES)
    {
        fatal_error("More than 10 charges");
    }

    if (total > MAX_NARRATIVES)
    {
        fatal_error("More than 15 total intact narratives");
    }
}


/*=========================================================
    LOAD MAPPINGS
=========================================================*/

void load_mappings(
    const char *filename,
    mapping_record mappings[],
    int *mapping_count
)
{
    FILE *fp;

    char line[256];

    char *token;

    fp = fopen(filename, "r");

    if (fp == NULL)
    {
        fatal_error("Unable to open mappings file");
    }

    *mapping_count = 0;

    while (fgets(line, sizeof(line), fp))
    {
        trim(line);

        if (strlen(line) == 0)
        {
            continue;
        }

        token = strtok(line, ",");

        if (token == NULL)
        {
            continue;
        }

        strncpy(
            mappings[*mapping_count].gloss_type,
            token,
            sizeof(mappings[*mapping_count].gloss_type) - 1
        );

        mappings[*mapping_count].gloss_type[
            sizeof(mappings[*mapping_count].gloss_type) - 1
        ] = '\0';


        token = strtok(NULL, ",");

        if (token == NULL)
        {
            fatal_error("Missing intact code in mapping file");
        }

        strncpy(
            mappings[*mapping_count].intact_code,
            token,
            sizeof(mappings[*mapping_count].intact_code) - 1
        );

        mappings[*mapping_count].intact_code[
            sizeof(mappings[*mapping_count].intact_code) - 1
        ] = '\0';


        printf(
            "MAP [%s] -> [%s]\n",
            mappings[*mapping_count].gloss_type,
            mappings[*mapping_count].intact_code
        );

        (*mapping_count)++;

        if (*mapping_count >= MAX_MAPPINGS)
        {
            fatal_error("Maximum mappings exceeded");
        }
    }

    printf(
        "Mappings loaded = %d\n",
        *mapping_count
    );

    fclose(fp);
}

/*=========================================================
    FIND MAPPING
=========================================================*/

mapping_record* find_mapping(
    mapping_record mappings[],
    int mapping_count,
    const char *gloss_type
)
{
    int i;

    printf(
        "\nLOOKING FOR [%s]\n",
        gloss_type
    );

    for (i = 0; i < mapping_count; i++)
    {
        if (strcmp(mappings[i].gloss_type, gloss_type) == 0)
        {
            printf("MATCH FOUND\n");

            return &mappings[i];
        }
    }

    printf("NO MATCH FOUND\n");

    return NULL;
}

/*=========================================================
    MAP TO INTACT
=========================================================*/

void map_to_intact(
    gloss_narrative narratives[],
    int narrative_count,
    gloss_charge charges[],
    int charge_count,
    mapping_record mappings[],
    int mapping_count,
    bps_intact_description *intact
)
{
    int i;

    mapping_record *mapping;

    char direction;

    strcpy(intact->segment_type, OUTPUT_SEGMENT_TYPE);

    strcpy(intact->delimiter, OUTPUT_DELIMITER);

    intact->narrative_count = 0;


    /* normal narratives */

    for (i = 0; i < narrative_count; i++)
    {
        mapping = find_mapping(
            mappings,
            mapping_count,
            narratives[i].narrative_type
        );

        if (mapping == NULL)
        {
            fatal_error("Narrative mapping not found");
        }

        strcpy(
            intact->narratives[intact->narrative_count].narrative_code,
            mapping->intact_code
        );

        /* destination comes from gloss input */

        intact->narratives[intact->narrative_count]
            .destination[0] = narratives[i].destination;

        intact->narratives[intact->narrative_count]
            .destination[1] = ' ';

        intact->narratives[intact->narrative_count]
            .destination[2] = '\0';

        strncpy(
            intact->narratives[intact->narrative_count].narrative_text,
            narratives[i].narrative_data,
            30
        );

        intact->narratives[intact->narrative_count]
            .narrative_text[30] = '\0';

        intact->narrative_count++;
    }


    /* charges */

    for (i = 0; i < charge_count; i++)
    {
        mapping = find_mapping(
            mappings,
            mapping_count,
            charges[i].charge_type
        );

        if (mapping == NULL)
        {
            fatal_error("Charge mapping not found");
        }

        switch (charges[i].direction)
        {
            case '+':
                direction = 'P';
                break;

            case '-':
                direction = 'N';
                break;

            case 'M':
                direction = 'M';
                break;

            default:
                fatal_error("Invalid charge direction");
        }

        strcpy(
            intact->narratives[intact->narrative_count].narrative_code,
            mapping->intact_code
        );

        /* charges have blank destination */

        strcpy(
            intact->narratives[intact->narrative_count].destination,
            "  "
        );

        sprintf(
            intact->narratives[intact->narrative_count].narrative_text,
            "%-3s%c%015lld",
            charges[i].charge_type,
            direction,
            charges[i].amount
        );

        intact->narrative_count++;
    }
}

/*=========================================================
    SORT
=========================================================*/

void sort_intact_narratives(
    intact_narrative narratives[],
    int count
)
{
    qsort(
        narratives,
        count,
        sizeof(intact_narrative),
        compare_narratives
    );
}

int compare_narratives(
    const void *a,
    const void *b
)
{
    const intact_narrative *na;
    const intact_narrative *nb;

    na = (const intact_narrative*)a;
    nb = (const intact_narrative*)b;

    return strcmp(
        na->narrative_code,
        nb->narrative_code
    );
}


/*=========================================================
    BUILD OUTPUT
=========================================================*/

void build_output_segment(
    bps_intact_description *intact,
    char *output
)
{
    int i;

    char temp[128];

    output[0] = '\0';

    sprintf(
        output,
        "%-2s%-4d%-20s",
        intact->segment_type,
        intact->narrative_count,
        ""
    );

    for (i = 0; i < intact->narrative_count; i++)
    {
        sprintf(
            temp,
            "%-3s %-2s    %-30s",
            intact->narratives[i].narrative_code,
            intact->narratives[i].destination,
            intact->narratives[i].narrative_text
        );

        strcat(output, temp);
    }

    strcat(output, intact->delimiter);
}


/*=========================================================
    WRITE OUTPUT
=========================================================*/

void write_output_file(
    const char *filename,
    const char *output
)
{
    FILE *fp;

    fp = fopen(filename, "w");

    if (fp == NULL)
    {
        fatal_error("Unable to open output file");
    }

    fprintf(fp, "%s\n", output);

    fclose(fp);
}