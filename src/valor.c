#include "valor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

const char *tipo_rp_nombre(TipoRP tipo) {
    switch (tipo) {
        case RP_CABAL: return "cabal";
        case RP_CON_VUELTO: return "con vuelto";
        case RP_CHISME: return "chisme";
        case RP_SIMONEL: return "simonel";
        case RP_NI_MAIZ: return "ni maiz";
        default: return "(desconocido)";
    }
}

ValorRP valor_copiar(const ValorRP *v) {
    ValorRP copia = *v;
    if (v->tipo == RP_CHISME) copia.dato.chisme = xstrdup(v->dato.chisme);
    return copia;
}

void valor_liberar(ValorRP *v) {
    if (v->tipo == RP_CHISME) {
        free(v->dato.chisme);
        v->dato.chisme = NULL;
    }
    v->tipo = RP_NI_MAIZ;
}

void valor_formatear(const ValorRP *v, char *buf, size_t n) {
    switch (v->tipo) {
        case RP_CABAL:
            snprintf(buf, n, "%lld", v->dato.cabal);
            break;
        case RP_CON_VUELTO:
            snprintf(buf, n, "%.15g", v->dato.con_vuelto);
            /* 12.0 se imprimiría "12"; se agrega ".0" para que siga viéndose decimal */
            if (!strpbrk(buf, ".en")) strncat(buf, ".0", n - strlen(buf) - 1);
            break;
        case RP_CHISME:
            snprintf(buf, n, "\"%s\"", v->dato.chisme);
            break;
        case RP_SIMONEL:
            snprintf(buf, n, "%s", v->dato.simonel ? "simon" : "nel");
            break;
        default:
            snprintf(buf, n, "ni maiz");
    }
}
