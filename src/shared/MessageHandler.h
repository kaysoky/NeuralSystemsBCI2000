////////////////////////////////////////////////////////////////////////////////
//
// File: MessageHandler.h
//
// Description: Facilities for centralized management of BCI2000 messages.
//
// Author: Juergen Mellinger
//
// Date:   Jul 24, 2003
//
////////////////////////////////////////////////////////////////////////////////
#ifndef MessageHandlerH
#define MessageHandlerH

#include <iostream>

class MessageHandler
{
  protected:
    MessageHandler() {}
    virtual ~MessageHandler() {}

  public:
    // Calling interface to the outside world.
    void HandleMessage( std::istream& );

  protected:
    // Message composing functions.
    // Accessible for inheritants only.
    template<typename content_type> static void PutMessage( std::ostream&, const content_type& );

  private:
    // Callback interface for inheritants to hook in. The return value indicates
    // whether the content was read from the stream.
    // Because template functions cannot be virtual we need to explicitly
    // enumerate all of them.
    // Note that the name of a handler should be Handle<ClassName> to
    // allow the CONSIDER macro to work.
    virtual bool HandleSTATUS(        std::istream& ) { return false; }
    virtual bool HandlePARAM(         std::istream& ) { return false; }
    virtual bool HandleSTATE(         std::istream& ) { return false; }
    virtual bool HandleGenericSignal( std::istream& ) { return false; }
    virtual bool HandleSTATEVECTOR(   std::istream& ) { return false; }
    virtual bool HandleSYSCMD(        std::istream& ) { return false; }

  public:
    enum { ignore = 1 << 7 };
  private:
    template<typename content_type> struct Header;
    template<> struct Header<class STATUS>
    { enum { desc = 1, supp = 0, src = ignore }; };
    template<> struct Header<class PARAM>
    { enum { desc = 2, supp = 0, src = ignore }; };
    template<> struct Header<class STATE>
    { enum { desc = 3, supp = 0, src = ignore }; };
    template<> struct Header<class GenericSignal>
    { enum { desc = 4, supp = 1, src = 0 }; };
    template<> struct Header<class STATEVECTOR>
    { enum { desc = 5, supp = 0, src = ignore }; };
    template<> struct Header<class SYSCMD>
    { enum { desc = 6, supp = 0, src = ignore }; };
};

#endif // MessageHandlerH

