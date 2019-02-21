#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/print.hpp>

using namespace eosio;

class [[eosio::contract("resource_delegating")]] resource_delegating : public eosio::contract {

public:
  using contract::contract;
  uint64_t autodelay = 60;
  
  resource_delegating(name receiver, name code,  datastream<const char*> ds): contract(receiver, code, ds) {}

  // Good code - commented out for 1.6.x release of EOSIO software
  // proper documentation for multiauth is missing.
  /*[[eosio::action]]
  void ask(name user) {
    require_auth( name("dexaraniiznx") );
    delegate_resources( user );
  }*/

  /*[[eosio::action]]
  void cancel(name user) {
    require_auth( name("dexaraniiznx") );
    undelegate_resources( user );
  }*/

  // Bad practice of multiauth implementation.
  [[eosio::action]]
  void ask(name user) {
    if (user == name("dexaraniiznx"))
    {
      require_auth(name("dexaraniiznx"));
    }
    else if (user == name("walletzzzzzz"))
    {
      require_auth(name("walletzzzzzz"));
    }
    else
    {
      require_auth(name(get_self()));
    }
    delegate_resources( user );
  }

  [[eosio::action]]
  void cancel(name user) {
    if (user == name("dexaraniiznx"))
    {
      require_auth(name("dexaraniiznx"));
    }
    else if (user == name("walletzzzzzz"))
    {
      require_auth(name("walletzzzzzz"));
    }
    else
    {
      require_auth(name(get_self()));
    }

    require_auth( name("dexaraniiznx") );
    undelegate_resources( user );
  }

private:

void undelegate_resources(name user){
  eosio::transaction t{};
  t.delay_sec = autodelay; // example - 1 minute delay for the tx

  uint64_t amount = 1;
  asset quantity(amount, symbol("EOS",4));

    t.actions.emplace_back(
      permission_level(name(get_self()), name("active")), 
      name("eosio"),
      name("undelegatebw"), 
      std::tuple(get_self(), user, quantity, quantity, false));

    t.send(now(), name(get_self())); // Send the transaction with some ID derived from the memo
  };

void delegate_resources(name user){
  uint64_t amount = 1;
  asset quantity(amount, symbol("EOS",4));
    action(
      permission_level(name(get_self()), name("active")), 
      name("eosio"),
      name("delegatebw"), 
      std::tuple(get_self(), user, quantity, quantity, false)
    ).send();
  };
};

EOSIO_DISPATCH( resource_delegating, (ask)(send)(cancel) )
