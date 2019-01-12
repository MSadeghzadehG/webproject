<template>
    <div>
        <h1 class="d-1 text-center">Typing Test</h1>
        <hr>
        <div class="container">
            <div class="row">
                <div class="col-md-12 text-center" v-html="outputHTML">
                    <!--<hr>-->
                    <!--<p class="text-justify">-->
                    <!--Hello World my name is <span class="label label-danger">Hadi</span> and i'm here to teach you front-end-->
                    <!--</p>-->
                </div>
            </div>
            <hr>
            <div class="row">
                <div class="col-md-12 text-center">
                    <textarea class="form-control" v-model="typedText"></textarea>
                </div>
            </div>
        </div>
    </div>
</template>

<script>
    export default {
        name: "TypingArea",
        data: function () {
            return {
                originalTestParagraph: 'Lorem ipsum dolor sit amet, consectetur adipiscing elit. In auctor pulvinar dolor, nec tempus sem iaculis non. Fusce sagittis vitae leo non varius. Nullam dapibus, lacus vel commodo varius, quam diam laoreet massa, dapibus euismod dolor nulla at justo. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Ut bibendum elit lacus, in feugiat odio viverra sed. Maecenas vestibulum mi nec aliquet tristique. In interdum euismod metus vel dapibus. Sed sodales magna et dictum imperdiet. Aenean magna odio, tempus quis consequat sed, feugiat ut elit. Morbi quis eleifend velit. Nunc venenatis nisi eget mi elementum, nec blandit odio maximus. Phasellus interdum mattis sodales.',
                typedText: '',
                typoIndex: -1,
            }
        },
        watch: {
            typedText: function (value) {
                for (let i = 0; i < value.length; i++) {
                    if (value[i] !== this.originalTestParagraph[i]) {
                        this.typoIndex = i;
                        break;
                    }
                    this.typoIndex = -1;
                }
            }
        },
        computed: {
            outputHTML: function () {
                let newHTML = '<p class="text-justify">';
                newHTML += '<span class="label label-success">';

                if (this.typoIndex === -1) {
                    newHTML += this.originalTestParagraph.substr(0, this.typedText.length) + '</span>';
                    newHTML += this.originalTestParagraph.substr(this.typedText.length)
                    newHTML += '</p>';
                    return newHTML;

                }
                newHTML += this.originalTestParagraph.substr(0, this.typoIndex) + '</span>';
                newHTML += '<span class="label label-danger">';
                newHTML += this.originalTestParagraph.substring(this.typoIndex, this.typedText.length);
                newHTML += '</span>';
                newHTML += this.originalTestParagraph.substr(this.typedText.length)
                newHTML += '</p>';
                return newHTML;

            }
        }
    }
</script>

<style>
    p {
        font-size: 15px !important;
        line-height: 30px;
    }

    .label {
        border-radius: 0 !important;
        font-size: 15px !important;
        padding: 0 !important;
        text-transform: none;
        font-weight: normal;
    }

    .label-danger {
        border-radius: 0 !important;
    }

    .label-success {
        border-radius: 0 !important;
    }
</style>
