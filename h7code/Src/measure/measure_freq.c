#include "stm32h7xx_hal.h"
#include "measure_freq.h"

void MeasureFrequencyStart(MeasureFreqData* data, int32_t count_calculate_mid, uint32_t sample_frequency)
{
    data->count_calculate_mid = count_calculate_mid;
    data->sample_frequency = sample_frequency;
    data->ysum = 0;
    data->ymax = 0;
    data->ymin = 0;
    data->ymid = 0;

    data->count_received_samples = 0;

    data->tstart = -1;
    data->tstop = -1;
    data->tcount = 0;

    data->prev_is_inited = false;
    data->prev_is_plus = false;
}


void MeasureFrequencyQuant(MeasureFreqData* data, int32_t sample)
{
    if(data->count_received_samples<data->count_calculate_mid)
    {
        if(data->count_received_samples==0)
            data->ymin = data->ymax = sample;
        int32_t y = sample;
        data->ysum += y;
        if(y<data->ymin)
            data->ymin = y;
        if(y>data->ymax)
            data->ymax = y;

        if(data->count_received_samples+1==data->count_calculate_mid)
        {
            data->ymid = (data->ysum+data->count_calculate_mid/2)/data->count_calculate_mid;
            float delta = 0.1;
            data->deltam = (int32_t)((data->ymax-data->ymin)*-delta)+data->ymid;
            data->deltap = (int32_t)((data->ymax-data->ymin)*delta)+data->ymid;
        }
    } else
    {
        int32_t y = sample;
        bool is_plus = (y-data->ymid)>0;
        if(y>data->deltam && y<data->deltap)
            goto End;

        if(!data->prev_is_inited)
        {
            data->prev_is_inited = true;
            data->prev_is_plus = is_plus;
            goto End;
        }

        if(data->prev_is_plus == is_plus)
            goto End;

        if(is_plus)
        {
            if(data->tstart==-1)
            {
                data->tstart = data->count_received_samples;
            } else
            {
                data->tstop = data->count_received_samples;
                data->tcount++;
            }
        }

        data->prev_is_plus = is_plus;
    }
End:;
    data->count_received_samples++;
}

float MeasureFrequencyValue(MeasureFreqData* data)
{
    if(data->tcount==0)
        return 0;
    return ((int64_t)data->tcount * (int64_t)data->sample_frequency)/(float)(data->tstop-data->tstart);
}
