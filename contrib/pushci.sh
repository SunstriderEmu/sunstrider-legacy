CIRCLECI_CLI_TOKEN=?
curl --user ${CIRCLECI_CLI_TOKEN}: \
     --form config=@.circleci/config.yml \
     https://circleci.com/api/v1.1/project/github/kelno/sunstrider-core/tree/master