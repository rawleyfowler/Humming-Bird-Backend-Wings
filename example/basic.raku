use Humming-Bird::Core;
use Humming-Bird::Backend::Wings;

get('/', sub ($request, $response) {
           return $response.html('Hello World from Wings!');
       });

listen(8080, backend => Humming-Bird::Backend::Wings);
