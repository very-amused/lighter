/*
 * xsct - X11 set color temperature
 *
 * Public domain, do as you wish.
 */

#include "xsct.h"

double DoubleTrim(double x, double a, double b)
{
    double buff[3] = {a, x, b};
    return buff[ (int)(x > a) + (int)(x > b) ];
}

struct temp_status get_sct_for_screen(Display *dpy, int screen, int icrtc, int fdebug)
{
    Window root = RootWindow(dpy, screen);
    XRRScreenResources *res = XRRGetScreenResourcesCurrent(dpy, root);

    int n, c;
    double t = 0.0;
    double gammar = 0.0, gammag = 0.0, gammab = 0.0, gammad = 0.0;
    struct temp_status temp;
    temp.temp = 0;
    temp.brightness = 1.0;

    n = res->ncrtc;
    if ((icrtc >= 0) && (icrtc < n))
        n = 1;
    else
        icrtc = 0;
    for (c = icrtc; c < (icrtc + n); c++)
    {
        RRCrtc crtcxid;
        int size;
        XRRCrtcGamma *crtc_gamma;
        crtcxid = res->crtcs[c];
        crtc_gamma = XRRGetCrtcGamma(dpy, crtcxid);
        size = crtc_gamma->size;
        gammar += crtc_gamma->red[size - 1];
        gammag += crtc_gamma->green[size - 1];
        gammab += crtc_gamma->blue[size - 1];

        XRRFreeGamma(crtc_gamma);
    }
    XFree(res);
    temp.brightness = (gammar > gammag) ? gammar : gammag;
    temp.brightness = (gammab > temp.brightness) ? gammab : temp.brightness;
    if (temp.brightness > 0.0 && n > 0)
    {
        gammar /= temp.brightness;
        gammag /= temp.brightness;
        gammab /= temp.brightness;
        temp.brightness /= n;
        temp.brightness /= BRIGHTHESS_DIV;
        temp.brightness = DoubleTrim(temp.brightness, 0.0, 1.0);
        if (fdebug > 0) fprintf(stderr, "DEBUG: Gamma: %f, %f, %f, brightness: %f\n", gammar, gammag, gammab, temp.brightness);
        gammad = gammab - gammar;
        if (gammad < 0.0)
        {
            if (gammab > 0.0)
            {
                t = exp((gammag + 1.0 + gammad - (GAMMA_K0GR + GAMMA_K0BR)) / (GAMMA_K1GR + GAMMA_K1BR)) + TEMPERATURE_ZERO;
            }
            else
            {
                t = (gammag > 0.0) ? (exp((gammag - GAMMA_K0GR) / GAMMA_K1GR) + TEMPERATURE_ZERO) : TEMPERATURE_ZERO;
            }
        }
        else
        {
            t = exp((gammag + 1.0 - gammad - (GAMMA_K0GB + GAMMA_K0RB)) / (GAMMA_K1GB + GAMMA_K1RB)) + (TEMPERATURE_NORM - TEMPERATURE_ZERO);
        }
    }
    else
        temp.brightness = DoubleTrim(temp.brightness, 0.0, 1.0);

    temp.temp = (int)(t + 0.5);

    return temp;
}

void sct_for_screen(Display *dpy, int screen, int icrtc, struct temp_status temp, int fdebug)
{
    double t = 0.0, b = 1.0, g = 0.0, gammar, gammag, gammab;
    int n, c;
    Window root = RootWindow(dpy, screen);
    XRRScreenResources *res = XRRGetScreenResourcesCurrent(dpy, root);

    t = (double)temp.temp;
    b = DoubleTrim(temp.brightness, 0.0, 1.0);
    if (temp.temp < TEMPERATURE_NORM)
    {
        gammar = 1.0;
        if (temp.temp > TEMPERATURE_ZERO)
        {
            g = log(t - TEMPERATURE_ZERO);
            gammag = DoubleTrim(GAMMA_K0GR + GAMMA_K1GR * g, 0.0, 1.0);
            gammab = DoubleTrim(GAMMA_K0BR + GAMMA_K1BR * g, 0.0, 1.0);
        }
        else
        {
            gammag = 0.0;
            gammab = 0.0;
        }
    }
    else
    {
        g = log(t - (TEMPERATURE_NORM - TEMPERATURE_ZERO));
        gammar = DoubleTrim(GAMMA_K0RB + GAMMA_K1RB * g, 0.0, 1.0);
        gammag = DoubleTrim(GAMMA_K0GB + GAMMA_K1GB * g, 0.0, 1.0);
        gammab = 1.0;
    }
    if (fdebug > 0) fprintf(stderr, "DEBUG: Gamma: %f, %f, %f, brightness: %f\n", gammar, gammag, gammab, b);
    n = res->ncrtc;
    if ((icrtc >= 0) && (icrtc < n))
        n = 1;
    else
        icrtc = 0;
    for (c = icrtc; c < (icrtc + n); c++)
    {
        int size, i;
        RRCrtc crtcxid;
        XRRCrtcGamma *crtc_gamma;
        crtcxid = res->crtcs[c];
        size = XRRGetCrtcGammaSize(dpy, crtcxid);

        crtc_gamma = XRRAllocGamma(size);

        for (i = 0; i < size; i++)
        {
            g = GAMMA_MULT * b * (double)i / (double)size;
            crtc_gamma->red[i] = (unsigned short int)(g * gammar + 0.5);
            crtc_gamma->green[i] = (unsigned short int)(g * gammag + 0.5);
            crtc_gamma->blue[i] = (unsigned short int)(g * gammab + 0.5);
        }

        XRRSetCrtcGamma(dpy, crtcxid, crtc_gamma);
        XRRFreeGamma(crtc_gamma);
    }

    XFree(res);
}
