/*
 * File:   CheckDecoding.cc
 * Author: rafopar
 *
 * Created on December 20, 2022, 3:11 PM
 */

#include <cstdlib>

#include <TH2D.h>
#include <TH1D.h>
#include <TFile.h>

// ===== Hipo headers =====
#include <reader.h>
#include <writer.h>
#include <dictionary.h>

#include <map>
#include <vector>
#include <utility>

using namespace std;

/*
 * Inputparameters are runnumber and file number
 */
int main(int argc, char** argv) {

    char outputFile[256];
    char inputFile[256];
    //Input parameters
    int run = 0;
    int fnum = -1;
    if (argc > 2) {
        run = atoi(argv[1]);
        fnum = atoi(argv[2]);
        //sprintf(inputFile, "%s", argv[1]);
        sprintf(inputFile, "Data/decoded_%d_%d.hipo", run, fnum);
        //Maybe we should change hipo outputfile accordingly to runnumber and filenumber
        sprintf(outputFile, "outSkim.hipo");
    } else {
        std::cout << " *** please provide a file name..." << std::endl;
        exit(0);
    }

    //standard hipo reader and dictionary
    hipo::reader reader;
    reader.open(inputFile);
    hipo::dictionary factory;
    reader.readDictionary(factory);
    //for debugging purposes
    factory.show();
    //get hipo event and banks
    hipo::event event;
    hipo::bank buRWellADC(factory.getSchema("URWELL::adc"));
    hipo::bank bRAWADc(factory.getSchema("RAW::adc"));
    hipo::bank bRunConf(factory.getSchema("RUN::config"));

    //counter to loop through events
    int evCounter = 0;

    const int crateID_fADC = 40; // the fADC is on ROC 40
    const int slot_fADC = 3; // and the slot is 3
    const int n_ts = 15; //number of time samples
    const int n_urwellapvs = 12;
    //128 long array to determine mapping of detector channel to APV channel for all 128 APV channels, if only half of the APV is connected only the first 64 entries are used in the array
    int AVPchannelmapping[128] = {
      127, 111, 95, 79, 63, 47, 31, 15, 123, 107, 91, 75, 59, 43, 27, 11, 119, 103, 87, 71, 55, 39, 23, 7, 115, 99, 83, 67, 51, 35, 19, 3,
      126, 110, 94, 78, 62, 46, 30, 14, 122, 106, 90, 74, 58, 42, 26, 10, 118, 102, 86, 70, 54, 38, 22, 6, 114, 98, 82, 66, 50, 34, 18, 2,
      125, 109, 93, 77, 61, 45, 29, 13, 121, 105, 89, 73, 57, 41, 25, 9,  117, 101, 85, 69, 53, 37, 21, 5, 113, 97, 81, 65, 49, 33, 17, 1,
      124, 108, 92, 76, 60, 44, 28, 12, 120, 104, 88, 72, 56, 40, 24, 8,  116, 100, 84, 68, 52, 36, 20, 4, 112, 96, 80, 64, 48, 32, 16, 0
    };


    //create output root file
    TFile *file_out = new TFile(Form("CheckDecoding_%d_%d.root", run, fnum), "Recreate");

    //FH comment APril 07: why ADCs histograms only from -1500 to 500 or 50 and not more negative
    //Definition of Histograms
    TH2D *h_ADC_chan = new TH2D("h_ADC_chan", "", 1711, -0.5, 1710.5, 400, -1500., 500.);
    TH2D *h_ADC_AllChan = new TH2D("h_ADC_AllChan", "", 1711, -0.5, 1710.5, 400, -1500., 50.);
    TH2D *h_ADC_Chan_ts_[n_ts];
    TH2D *h_ch_Coorelation = new TH2D("h_ch_Coorelation", "", 17, -0.5, 16.5, 17, -0.5, 16.5);
    TH2D *h_ADC_TopBot1 = new TH2D("h_ADC_TopBot1", "", 200, 0., 500, 200, 0., 500);

    TH2D *h2_uniqueid_vs_layer = new TH2D("h2_uniqueid_vs_layer", "", 1711, -0.5, 1710.5, 3, -0.5, 2.5);
    TH2D *h2_uniqueid_vs_component = new TH2D("h2_uniqueid_vs_component", "", 1711, -0.5, 1710.5, 705, -0.5, 704.5);
    TH2D *h2_uniqueid_vs_apvslot = new TH2D("h2_uniqueid_vs_apvslot", "", 1711, -0.5, 1710.5, 13, -1.5, 12.5);
    TH2D *h2_apvslot_vs_component = new TH2D("h2_apvslot_vs_component", "", 13, -1.5, 12.5, 705, -0.5, 704.5);
    TH2D *h2_apvslot_vs_layer = new TH2D("h2_apvslot_vs_layer", "", 13, -1.5, 12.5, 3, -0.5, 2.5);

    //these two histograms were just for checking
    //TH2D *h2_apvchannel_vs_channel_slot0 = new TH2D("h2_apvchannel_vs_channel_slot0", "", 128, -0.5, 127.5, 705, -0.5, 704.5);
    //TH2D *h2_apvchannel_vs_channel_slot1 = new TH2D("h2_apvchannel_vs_channel_slot1", "", 128, -0.5, 127.5, 705, -0.5, 704.5);
    //Create histogram for each time sample
    for (int i = 0; i < n_ts; i++) {
        h_ADC_Chan_ts_[i] = new TH2D(Form("h_ADC_Chan_ts_%d", i), "", 1711, -0.5, 1710.5, 400, -1500., 500.);
    }
    TH2D *h2_ADC_Chan_apvslot[n_urwellapvs];
    for (int j = 0; j < n_urwellapvs; j++) {
        h2_ADC_Chan_apvslot[j] = new TH2D(Form("h2_ADC_Chan_apvslot_%d", j), Form("ADC average per APV channel for slot %d", j), 128, -0.5, 127., 400, -1500., 500.);
    }


    /**
     * Checking some decoding staff
     */
    const int n_testEv = 2;
    const int n_testCh = 3;
    //eventloop in try block
    try {

        while (reader.next() == true) {
            reader.read(event);
            //increment evCounter
            evCounter = evCounter + 1;

            //only process 2000 events
            if( evCounter > 2000 ){break;}

            if (evCounter % 1000 == 0) {
                cout.flush() << "Processed " << evCounter << " events \r";
            }

            event.getStructure(buRWellADC);
            event.getStructure(bRAWADc);
            event.getStructure(bRunConf);
            int n_ADC = buRWellADC.getRows();
            int n_RunConf = bRunConf.getRows();
            int n_fADC = bRAWADc.getRows();

            /**
             * Only 4 channels on fADC are read out 0,1 and 9,10 . 0 and 1 are from Bottom scintillator, while 9 and 10 are from Top
             */
             //Comment FH 04/07/2023 I think the channels for the scintillators have to be modified
            int ADC_SCTop = 0;
            int ADC_SCBot = 0;

            for (int ifADC = 0; ifADC < n_fADC; ifADC++) {

                int crate = bRAWADc.getInt("crate", ifADC);
                int slot = bRAWADc.getInt("slot", ifADC);

                if (!(crate == crateID_fADC && slot == slot_fADC)) {
                    cout << "Wrong crate or slot. Should not be happen" << endl;
                    cout << " crate = " << crate << "     slot = " << slot << endl;

                    continue;
                }

                int ch = bRAWADc.getInt("channel", ifADC);
                int ADC = bRAWADc.getInt("ADC", ifADC);
                double time = bRAWADc.getFloat("time", ifADC);

                if (ch == 0 || ch == 1) {
                    ADC_SCBot = ADC_SCBot + ADC;
                } else if (ch == 9 || ch == 10) {
                    ADC_SCTop = ADC_SCTop + ADC;
                }
                //FH April 07: I am not sure I understand this loop because it would mix separate scinitllators but not importatnt at the moment
                for( int jfADC = ifADC + 1; jfADC < n_fADC; jfADC++ ){

                    int jch = bRAWADc.getInt("channel", jfADC);
                    int jADC = bRAWADc.getInt("ADC", jfADC);

                    if( jADC > 0 && ADC > 0 ){
                        h_ch_Coorelation->Fill(ch, jch);
                    }
                }

            }

            h_ADC_TopBot1->Fill(ADC_SCBot, ADC_SCTop);

            std::map< int, int > m_ADC_[n_ts]; // The key of the map is unique_channel, and the value is the ADC value
            std::map< int, int > m_APVslot; // The key of the map is unique_channel, and the value is the APVslot
            std::map< int, int > m_APVchannel; // The key of the map is unique_channel, and the value is the APV channel

            for (int i = 0; i < n_ADC; i++) {
                int sector = buRWellADC.getInt("sector", i);
                int layer = buRWellADC.getInt("layer", i); //1 for U and 2 for V
                int channel = buRWellADC.getInt("component", i); //strip ID 1 to 704
                int ADC = buRWellADC.getInt("ADC", i); //ADC value itself
                int uniqueChan = int(buRWellADC.getFloat("time", i));
                int ts = buRWellADC.getInt("ped", i); //Timesample

                int slot = layer; //U or V

                if (evCounter < 5 && layer < 3) {
              //      cout << evCounter << "\t" << slot << "\t" << ts << "\t" << channel << "\t" << ADC << endl;
                }
                /*APV slot and components U
                0   1-64
                1   65-192
                2   193-320
                4   321-448
                9   449-576
                11  577-704
                APV slot and components V
                6   1-64
                7   65-192
                8   193-320
                10  321-448
                3   449-576
                5   577-704
              */
                //if (sector == 85 && layer == 1) { // Non Sec 6 events are not physical
                if (sector == 6) { // Non Sec 6 events are not physical
                    h_ADC_Chan_ts_[ts]->Fill(uniqueChan, ADC);
                    h_ADC_AllChan->Fill(uniqueChan, ADC);
                    m_ADC_[ts][uniqueChan] = ADC;
                    h2_uniqueid_vs_layer->Fill(uniqueChan, layer);
                    h2_uniqueid_vs_component->Fill(uniqueChan, channel);
                    int APVslot = -1;
                    int APVchannel = -1;
                    //Assign APV slot
                    if (layer == 1) { //U strips
                      if (channel >=1 && channel <=64) {
                        APVslot = 0;
                        APVchannel = AVPchannelmapping[channel-1];
                      }
                      else if (channel >=65 && channel <=192) {
                        APVslot = 1;
                        APVchannel = AVPchannelmapping[channel-65];
                      }
                      else if (channel >=193 && channel <=320) {
                        APVslot = 2;
                        APVchannel = AVPchannelmapping[channel-193];
                      }
                      else if (channel >=321 && channel <=448) {
                        APVslot = 4;
                        APVchannel = AVPchannelmapping[channel-321];
                      }
                      else if (channel >=449 && channel <=576) {
                        APVslot = 9;
                        APVchannel = AVPchannelmapping[channel-449];
                      }
                      else if (channel >=577 && channel <=704) {
                        APVslot = 11;
                        APVchannel = AVPchannelmapping[channel-577];
                      }
                      else {
                        APVslot = -1;
                        cout << "WARNING: no assignment of APVslot for urwell layer " << layer << " and channel " << channel << endl;
                      }
                    }
                    if (layer == 2) { //V strips
                      if (channel >=1 && channel <=64) {
                        APVslot = 6;
                        APVchannel = AVPchannelmapping[channel-1];
                      }
                      else if (channel >=65 && channel <=192) {
                        APVslot = 7;
                        APVchannel = AVPchannelmapping[channel-65];
                      }
                      else if (channel >=193 && channel <=320) {
                        APVslot = 8;
                        APVchannel = AVPchannelmapping[channel-193];
                      }
                      else if (channel >=321 && channel <=448) {
                        APVslot = 10;
                        APVchannel = AVPchannelmapping[channel-321];
                      }
                      else if (channel >=449 && channel <=576) {
                        APVslot = 3;
                        APVchannel = AVPchannelmapping[channel-449];
                      }
                      else if (channel >=577 && channel <=704) {
                        APVslot = 5;
                        APVchannel = AVPchannelmapping[channel-577];
                      }
                      else {
                        APVslot = -1;
                        cout << "WARNING: no assignment of APVslot for urwell layer " << layer << " and channel/component " << channel << endl;
                      }
                    }
                    h2_apvslot_vs_component->Fill(APVslot, channel);
                    h2_apvslot_vs_layer->Fill(APVslot, layer);
                    h2_uniqueid_vs_apvslot->Fill(uniqueChan, APVslot);
                  //  h2_ADC_Chan_apvslot[APVslot]->Fill(APVchannel, ADC);
                    m_APVslot[uniqueChan] = APVslot;
                    m_APVchannel[uniqueChan] = APVchannel;


                }
            }

           for (auto map0 : m_ADC_[0]) {
                int ch = map0.first;
                //double avg_ADC = (m_ADC_[0][ch] + m_ADC_[1][ch] + m_ADC_[2][ch]) / 3.;
                double avg_ADC = 0.;
                for( int i_ts = 0; i_ts < n_ts; i_ts++ ){
                    avg_ADC = avg_ADC + m_ADC_[i_ts][ch];
                }
                avg_ADC = avg_ADC/double(n_ts);
                h_ADC_chan->Fill(ch, avg_ADC);
                h2_ADC_Chan_apvslot[m_APVslot[ch]]->Fill(m_APVchannel[ch], avg_ADC);
            }

        }
    } catch (const char msg) {
        cerr << msg << endl;
    }

    gDirectory->Write();
    file_out->Close();
    return 0;
}
