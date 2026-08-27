/*
 * Blind, semantic-only PlotEx rewrite.
 *
 * This file deliberately contains no names taken from a generated or upstream
 * implementation.  Integration replaces every @...@ token with a host type,
 * constant, function, or adapter macro.  The canonical plot_ex_body.c.in is
 * the host-adapted form of this blind contract, not a textual include for this
 * abstract signature.
 *
 * Host API audit invariants:
 * - Register the item with the complete label-derived ID and the NoNav flag;
 *   use that same ID for hover testing.
 * - Clamp the horizontal hover fraction to [0.0f, 0.999f].
 */

#include <float.h>

int
@PLOT_EX_FUNCTION@(@CONTEXT_TYPE@ *ctx,
                   @PLOT_TYPE@ type,
                   const char *label,
                   float (*get)(void *, int),
                   void *data,
                   int count,
                   int offset,
                   const char *overlay,
                   float scale_min,
                   float scale_max,
                   const @VECTOR_TYPE@ *requested_size);
