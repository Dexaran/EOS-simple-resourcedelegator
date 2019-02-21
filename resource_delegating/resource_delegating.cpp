#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/print.hpp>

using namespace eosio;

class [[eosio::contract("example")]] example_contract : public eosio::contract {

public:
  using contract::contract;
  //uint32_t standard_delay = 24 * 60 * 60; // 1 day in seconds
  uint32_t standard_delay         = 180; // 3 minute in seconds for testing purpose only
  uint64_t current_queue_position = 0;
  uint64_t last_queue_position    = 0;
  
  example_contract(name receiver, name code,  datastream<const char*> ds): contract(receiver, code, ds) {}


  struct [[eosio::table]] request {
    name username;
    uint64_t CPU;
    uint64_t NET;
    uint64_t position_in_queue;

    uint64_t primary_key() const { return username.value; }

    EOSLIB_SERIALIZE( request, (username)(CPU)(NET)(position_in_queue))
  };

  typedef eosio::multi_index<name("request"), request> requests;

  [[eosio::action]]
  void ask(name user, uint64_t _CPU, uint64_t _NET) {

    multiauth(user);

    requests reqest_instance(_code, _code.value);
    auto iterator = reqest_instance.find(user.value);   
    if( iterator == reqest_instance.end() )
    {
      reqest_instance.emplace(user, [&]( auto& row ) {
        row.username = user;
        row.CPU = _CPU;
        row.NET = _NET;
        row.position = last_position;
        last_position++;
      });
      delegate_resources( user, _CPU, _NET, now() + standard_delay);
      printmsg(user, "delegated CPU and NET by @Dexaran");
    }
    else {
      return;
    }
  }

  [[eosio::action]]
  void notify(name user, std::string msg) {
    require_auth(get_self());
    require_recipient(user);
  }

  [[eosio::action]]
  void eraserecord(name user){
    require_auth(get_self());
    requests reqest_instance(_code, _code.value);
    auto iterator = reqest_instance.find(user.value);
    eosio_assert(iterator != reqest_instance.end(), "Record does not exist");
    reqest_instance.erase(iterator);
  };

private:

  void printmsg(name user, std::string _msg) {
    action(
      permission_level(name(get_self()), name("active")),
      get_self(),
      name("notify"),
      std::make_tuple(user, _msg)
    ).send();
  }

// Bad practice of multiauth implementation.
void multiauth(name user)
{
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
};

void delegate_resources(name user, uint64_t _CPU, uint64_t _NET, uint64_t _timeframe){

  asset CPU_delegated(_CPU, symbol("EOS",4));
  asset NET_delegated(_NET, symbol("EOS",4));
  action(
    permission_level(name(get_self()), name("active")), 
    name("eosio"),
    name("delegatebw"), 
    std::tuple(get_self(), user, NET_delegated, CPU_delegated, false)
  ).send();

  eosio::transaction tx{};
  tx.delay_sec = standard_delay;

  tx.actions.emplace_back(
      permission_level(name(get_self()), name("active")), 
      name("eosio"),
      name("undelegatebw"), 
      std::tuple(get_self(), user, NET_delegated, CPU_delegated, false));

  tx.actions.emplace_back(
      permission_level(name(get_self()), name("active")),
      get_self(),
      name("notify"),
      std::make_tuple(user, "deferred transaction: automated undelegation of bandwidth in 3 minutes"));

  tx.actions.emplace_back(
      permission_level(name(get_self()), name("active")),
      get_self(),
      name("eraserecord"),
      std::make_tuple(user));

  tx.send(now(), name(get_self()));
  };
};

EOSIO_DISPATCH( example_contract, (ask)(notify) )
