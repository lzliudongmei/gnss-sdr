/*!
 * \file galileo_e1_pcps_quicksync_ambiguous_acquisition.cc
 * \brief Adapts a PCPS acquisition block to an AcquisitionInterface for
 *  Galileo E1 Signals using the QuickSync Algorithm
 * \author Damian Miralles, 2014. dmiralles2009@gmail.com
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2014  (see AUTHORS file for a list of contributors)
 *
 * GNSS-SDR is a software defined Global Navigation
 *          Satellite Systems receiver
 *
 * This file is part of GNSS-SDR.
 *
 * GNSS-SDR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GNSS-SDR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNSS-SDR. If not, see <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 */

#include "galileo_e1_pcps_quicksync_ambiguous_acquisition.h"
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/math/distributions/exponential.hpp>
#include <glog/logging.h>
#include <cmath>
#include "galileo_e1_signal_processing.h"
#include "Galileo_E1.h"
#include "configuration_interface.h"

using google::LogMessage;

GalileoE1PcpsQuickSyncAmbiguousAcquisition::GalileoE1PcpsQuickSyncAmbiguousAcquisition(
        ConfigurationInterface* configuration, std::string role,
        unsigned int in_streams, unsigned int out_streams,
        boost::shared_ptr<gr::msg_queue> queue) :
               role_(role), in_streams_(in_streams), out_streams_(out_streams), queue_(queue)
{
    configuration_ = configuration;
    std::string default_item_type = "gr_complex";
    std::string default_dump_filename = "../data/acquisition.dat";

    DLOG(INFO) << "role " << role;

    item_type_ = configuration_->property(role + ".item_type",
            default_item_type);

    fs_in_ = configuration_->property("GNSS-SDR.internal_fs_hz", 4000000);
    if_ = configuration_->property(role + ".ifreq", 0);
    dump_ = configuration_->property(role + ".dump", false);
    shift_resolution_ = configuration_->property(role + ".doppler_max", 15);
    sampled_ms_ = configuration_->property(role + ".coherent_integration_time_ms", 8);

    /*--- Find number of samples per spreading code (4 ms)  -----------------*/
    code_length_ = round(
            fs_in_
            / (Galileo_E1_CODE_CHIP_RATE_HZ
                    / Galileo_E1_B_CODE_LENGTH_CHIPS));

    int samples_per_ms = round(code_length_ / 4.0);


    /*Calculate the folding factor value based on the formula described in the paper.
    This may be a bug, but acquisition also work by variying the folding factor at va-
    lues different that the expressed in the paper. In adition, it is important to point
    out that by making the folding factor smaller we were able to get QuickSync work with 
    Galileo. Future work should be directed to test this asumption statistically.*/
    
    //folding_factor_ = (unsigned int)ceil(sqrt(log2(code_length_)));
    folding_factor_ = configuration_->property(role + ".folding_factor", 2);
    
    if (sampled_ms_ % (folding_factor_*4) != 0)
        {
            LOG(WARNING) << "QuickSync Algorithm requires a coherent_integration_time"
                    << " multiple of "<<(folding_factor_*4)<<"ms, Value entered "
                    <<sampled_ms_<<" ms";

            if(sampled_ms_ < (folding_factor_*4))
                {
                    sampled_ms_ = (int) (folding_factor_*4);
                }
            else
                {
                    sampled_ms_ = (int)(sampled_ms_/(folding_factor_*4)) * (folding_factor_*4);
                }
            LOG(WARNING) << "coherent_integration_time should be multiple of "
                    << "Galileo code length (4 ms). coherent_integration_time = "
                    << sampled_ms_ << " ms will be used.";

        }
   // vector_length_ = (sampled_ms_/folding_factor_) * code_length_;
	vector_length_ = sampled_ms_ * samples_per_ms;
    bit_transition_flag_ = configuration_->property(role + ".bit_transition_flag", false);

    if (!bit_transition_flag_)
        {
            max_dwells_ = configuration_->property(role + ".max_dwells", 1);
        }
    else
        {
            max_dwells_ = 2;
        }

    dump_filename_ = configuration_->property(role + ".dump_filename",
            default_dump_filename);

    code_ = new gr_complex[code_length_];
    LOG(INFO) <<"Vector Length: "<<vector_length_
            <<", Samples per ms: "<<samples_per_ms
            <<", Folding factor: "<<folding_factor_
            <<", Sampled  ms: "<<sampled_ms_
            <<", Code Length: "<<code_length_;
    if (item_type_.compare("gr_complex") == 0)
        {
            item_size_ = sizeof(gr_complex);
            acquisition_cc_ = pcps_quicksync_make_acquisition_cc(folding_factor_,
                    sampled_ms_, max_dwells_, shift_resolution_, if_, fs_in_,
                    samples_per_ms, code_length_, bit_transition_flag_, queue_,
                    dump_, dump_filename_);
            stream_to_vector_ = gr::blocks::stream_to_vector::make(item_size_,
                    vector_length_);
            DLOG(INFO) << "stream_to_vector_quicksync("
                    << stream_to_vector_->unique_id() << ")";
            DLOG(INFO) << "acquisition_quicksync(" << acquisition_cc_->unique_id()
                           << ")";
        }
    else
        {
            LOG(WARNING) << item_type_
                    << " unknown acquisition item type";
        }
}


GalileoE1PcpsQuickSyncAmbiguousAcquisition::~GalileoE1PcpsQuickSyncAmbiguousAcquisition()
{
    delete[] code_;
}


void
GalileoE1PcpsQuickSyncAmbiguousAcquisition::set_channel(unsigned int channel)
{
    channel_ = channel;
    if (item_type_.compare("gr_complex") == 0)
        {
            acquisition_cc_->set_channel(channel_);
        }
}


void
GalileoE1PcpsQuickSyncAmbiguousAcquisition::set_threshold(float threshold)
{

    float pfa = configuration_->property(role_+ boost::lexical_cast<std::string>(channel_) + ".pfa", 0.0);

    if(pfa==0.0) pfa = configuration_->property(role_+".pfa", 0.0);

    if(pfa==0.0)
        {
            threshold_ = threshold;
        }
    else
        {
            threshold_ = calculate_threshold(pfa);
        }

    DLOG(INFO) <<"Channel "<<channel_<<" Threshold = " << threshold_;

    if (item_type_.compare("gr_complex") == 0)
        {
            acquisition_cc_->set_threshold(threshold_);
        }
}


void
GalileoE1PcpsQuickSyncAmbiguousAcquisition::set_doppler_max(unsigned int doppler_max)
{
    doppler_max_ = doppler_max;

    if (item_type_.compare("gr_complex") == 0)
        {
            acquisition_cc_->set_doppler_max(doppler_max_);
        }
}


void
GalileoE1PcpsQuickSyncAmbiguousAcquisition::set_doppler_step(unsigned int doppler_step)
{
    doppler_step_ = doppler_step;
    if (item_type_.compare("gr_complex") == 0)
        {
            acquisition_cc_->set_doppler_step(doppler_step_);
        }
}


void
GalileoE1PcpsQuickSyncAmbiguousAcquisition::set_channel_queue(
        concurrent_queue<int> *channel_internal_queue)
{
    channel_internal_queue_ = channel_internal_queue;
    if (item_type_.compare("gr_complex") == 0)
        {
            acquisition_cc_->set_channel_queue(channel_internal_queue_);
        }
}


void
GalileoE1PcpsQuickSyncAmbiguousAcquisition::set_gnss_synchro(
        Gnss_Synchro* gnss_synchro)
{
    gnss_synchro_ = gnss_synchro;
    if (item_type_.compare("gr_complex") == 0)
        {
            acquisition_cc_->set_gnss_synchro(gnss_synchro_);
        }
}


signed int
GalileoE1PcpsQuickSyncAmbiguousAcquisition::mag()
{
    if (item_type_.compare("gr_complex") == 0)
        {
            return acquisition_cc_->mag();
        }
    else
        {
            return 0;
        }
}


void
GalileoE1PcpsQuickSyncAmbiguousAcquisition::init()
{
    acquisition_cc_->init();
    set_local_code();
}


void
GalileoE1PcpsQuickSyncAmbiguousAcquisition::set_local_code()
{
    if (item_type_.compare("gr_complex") == 0)
        {
            bool cboc = configuration_->property(
                    "Acquisition" + boost::lexical_cast<std::string>(channel_)
                    + ".cboc", false);

            std::complex<float> * code = new std::complex<float>[code_length_];

            galileo_e1_code_gen_complex_sampled(code, gnss_synchro_->Signal,
                    cboc, gnss_synchro_->PRN, fs_in_, 0, false);

           
           for (unsigned int i = 0; i < (sampled_ms_/(folding_factor_*4)); i++)
               {
                   memcpy(&(code_[i*code_length_]), code,
                          sizeof(gr_complex)*code_length_);
               }
            
           // memcpy(code_, code,sizeof(gr_complex)*code_length_);
            acquisition_cc_->set_local_code(code_);

            delete[] code;
            code = NULL;
        }
}


void
GalileoE1PcpsQuickSyncAmbiguousAcquisition::reset()
{
    if (item_type_.compare("gr_complex") == 0)
        {
            acquisition_cc_->set_active(true);
        }
}

float GalileoE1PcpsQuickSyncAmbiguousAcquisition::calculate_threshold(float pfa)
{
    unsigned int frequency_bins = 0;
    for (int doppler = (int)(-doppler_max_); doppler <= (int)doppler_max_; doppler += doppler_step_)
        {
            frequency_bins++;
        }

    DLOG(INFO) <<"Channel "<<channel_<<"  Pfa = "<< pfa;

    unsigned int ncells = code_length_/folding_factor_ * frequency_bins;
    double exponent = 1 / (double)ncells;
    double val = pow(1.0 - pfa, exponent);
    double lambda = double(code_length_/folding_factor_);
    boost::math::exponential_distribution<double> mydist (lambda);
    float threshold = (float)quantile(mydist,val);

    return threshold;
}


void
GalileoE1PcpsQuickSyncAmbiguousAcquisition::connect(gr::top_block_sptr top_block)
{
    if (item_type_.compare("gr_complex") == 0)
        {
            top_block->connect(stream_to_vector_, 0, acquisition_cc_, 0);
        }
}


void
GalileoE1PcpsQuickSyncAmbiguousAcquisition::disconnect(gr::top_block_sptr top_block)
{
    if (item_type_.compare("gr_complex") == 0)
        {
            top_block->disconnect(stream_to_vector_, 0, acquisition_cc_, 0);
        }
}


gr::basic_block_sptr GalileoE1PcpsQuickSyncAmbiguousAcquisition::get_left_block()
{
    return stream_to_vector_;
}


gr::basic_block_sptr GalileoE1PcpsQuickSyncAmbiguousAcquisition::get_right_block()
{
    return acquisition_cc_;
}

