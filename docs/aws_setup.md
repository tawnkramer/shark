# Setting up for AWS EC2 training #

if you plan on using aws to train, you need to install and setup a few things.

On the raspberrypi or wherever you will launch training, you need to install and setup awscli.

1. from the command line, run: pip install awscli
2. log into aws console and generate some access tokens in the [IAM]( 
    https://console.aws.amazon.com/iam/home?region=us-east-1#/home)
3. create a user and assign an access policy with premissions for EC2
4. from the command line, run: aws configure
    * add the AWS Access Key ID 
    * add the AWS Secret Access Key 
    * specify the region. I recommend us-east-1 for cheapest p2 large spot instances.
    * accept default output - should be json.

5. copy aws_config_example.json to aws_config.json
6. modify aws_config.json to match your preferences. You will need to change the KeyName, SecurityGroupIds at a minimum. If you chose a different region, you will need to find an ami that has Keras/Tensorflow pre-installed.
7. look at the config.json ec2 settings section and make changes as needed.
