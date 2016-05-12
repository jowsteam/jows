#!/usr/bin/env python
import xml.etree.ElementTree as ET
import sys

class QosTresholds (object):
    def  __init__(self):
        self._weighted_treshold = dict()
    
    def set_mean_treshold(self, mean_treshold):
        self._mean = mean_treshold

    def get_mean(self):
        return self._mean

    def set_weighted_treshold(self, treshold, percentage):
        self._weighted_treshold[treshold] = percentage

    def get_wieghted_tresholds(self):
        return self._weighted_treshold

class QosClassifier (object):
    def __init__(self):
        self.delay_tresholds = QosTresholds()
        self.jitter_tresholds = QosTresholds()

    def histogram_suitable(self, histogram, treshold):
        passes = True
        count = 0
        value_sum = 0
        for k,v in histogram.iteritems():
            count += v # number of total packets
            value_sum += k*v # weighted value with number packets
        try:
            mean = value_sum/count
            if mean > treshold.get_mean():
                passes = False
#            print "Mean: %.4f\tTreshold: %.4f" % (mean, treshold.get_mean())
        except:
            pass
        return passes
class VoiceQosClassifier (QosClassifier):
    def __init__(self):
        super(VoiceQosClassifier, self).__init__()
        self.delay_tresholds.set_mean_treshold(0.1)
        self.jitter_tresholds.set_mean_treshold(0.03)
class VideoQosClassifier (QosClassifier):
    def __init__(self):
        super(VideoQosClassifier, self).__init__()
        self.delay_tresholds.set_mean_treshold(0.3)
#        self.jitter_tresholds.set_mean_treshold(10)

class FlowObject(object):
    ''' Manages flow object, 
    classifies (using QosClassifier objects) if suitable for given QoS application
    '''
    def __init__(self, xml_flow_element):
        self._xml = xml_flow_element

    @property
    def id(self):
        return int(self._xml.attrib['flowId'])

    @property
    def delay_hist(self):
        return self._xml.find('delayHistogram')
    @property
    def delay_dict(self):
        delay_dict = dict()
        for bbin in self.delay_hist.findall('bin'):
            # put in the middle of the bin for simplicity
            key = float(bbin.attrib['start']) + float(bbin.attrib['width'])/2
            value = int(bbin.attrib['count'])
            delay_dict[key] = value
        return delay_dict

    @property
    def jitter_hist(self):
        return self._xml.find('jitterHistogram')
    @property
    def jitter_dict(self):
        jitter_dict = dict()
        for bbin in self.jitter_hist.findall('bin'):
            # put in the middle of the bin for simplicity
            key = float(bbin.attrib['start']) + float(bbin.attrib['width'])/2
            value = int(bbin.attrib['count'])
            jitter_dict[key] = value
        return jitter_dict

    @property
    def tx_packets(self):
        return int(self._xml.attrib['txPackets'])
    @property
    def rx_packets(self):
        return int(self._xml.attrib['rxPackets'])
    @property
    def lost_packets(self):
        return int(self._xml.attrib['lostPackets'])

    @property
    def packet_loss(self):
        if self.tx_packets<1:
            return 0.0
        return float(self.tx_packets - self.rx_packets)/self.tx_packets
        

    def add_5tuple(self, xml_5tuple):
        self._5tuple = xml_5tuple

    def suitable_for_qos(self, qos_classifier):
        passes = True
        if not self.delay_suitable(qos_classifier):
            passes = False
        if not self.jitter_suitable(qos_classifier):
            passes = False
        
        return passes

    def jitter_suitable(self, qos_classifier):
#        print '| Testing jitter: '
        return qos_classifier.histogram_suitable(self.jitter_dict, qos_classifier.jitter_tresholds)
    def delay_suitable(self, qos_classifier):
#        print '| Testing delay: '
        return qos_classifier.histogram_suitable(self.delay_dict, qos_classifier.delay_tresholds)


if __name__=="__main__":
    tree = ET.parse(sys.argv[1])
    root = tree.getroot()
    flows = []

    for flow_xml in root.iter('Flow'):
        if flow_xml.attrib.has_key('txPackets'):
            flows.append(FlowObject(flow_xml))
        else:
            for f in flows:
                if flow_xml.attrib['flowId'] == f.id:
                    f.add_5tuple(flow_xml)
                    break
    voice = VoiceQosClassifier()
    video = VideoQosClassifier()
    interesting_flows = flows[6:]
    for f in interesting_flows:
        print '=== Flow %d ===' % (f.id)
#        print '| packet loss: %.2f%s' %(100*f.packet_loss, '%')
#        print '## SUITABLE FOR VOICE? ##'
        if f.suitable_for_qos(voice): print 'OK'
        else: print 'NOT OK'
#        print '## SUITABLE FOR VIDEO? ##'
        if f.suitable_for_qos(video): print 'OK'
        else: print 'NOT OK'
