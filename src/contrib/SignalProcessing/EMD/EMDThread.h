#ifndef EMDTHREAD_H
#define EMDTHREAD_H

// To avoid zero values while calculating the standard deviation
#define EPSILON 0.0000001f

#include "ThreadedFilter.h"

class EMDThread : public FilterThread {
	public:
		EMDThread() : siftStopSD( 0.3 ), envelopeInterpolation( cubic ), maxIMFs( 10 ) {}
		~EMDThread() {}
		
	protected:
		void OnPublish() const;
		void OnPreflight( const SignalProperties&, SignalProperties& ) const;
		void OnInitialize( const SignalProperties&, const SignalProperties& );
		void OnProcess( const GenericSignal&, GenericSignal& );

	private:
		double siftStopSD;
		enum interpolationMethod {
			linear = 0,
			cubic,
	    } envelopeInterpolation;
		int maxIMFs;

		bool SingleEMD( GenericSignal&, const int );
		bool SingleSift( const std::vector<double>&, std::vector<double>* );

		void CubicSplineInterpolate( const std::vector<int>&, const std::vector<double>&, std::vector<double>* );
};
#endif // EMDTHREAD_H
